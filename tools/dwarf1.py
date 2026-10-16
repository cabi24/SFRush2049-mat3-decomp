"""Decode the DWARF v1 (.debug/.line) info in rush_d.elf and attach source files to symbols.

usage: dwarf1.py <rush_d.elf> <out_dir> [splits.txt]
Writes: rush_d_symbols.csv (rewritten with source columns), rush_d_compile_units.csv,
        rush_d_functions.csv, rush_d_variables.csv
"""
import sys, os, struct, csv, bisect, collections
sys.path.insert(0, os.path.dirname(__file__))
from elfio import Elf

TAGS = {0x00: 'padding', 0x01: 'array_type', 0x02: 'class_type', 0x03: 'entry_point', 0x04: 'enumeration_type',
        0x05: 'formal_parameter', 0x06: 'global_subroutine', 0x07: 'global_variable', 0x0a: 'label',
        0x0b: 'lexical_block', 0x0c: 'local_variable', 0x0d: 'member', 0x0f: 'pointer_type',
        0x10: 'reference_type', 0x11: 'compile_unit', 0x12: 'string_type', 0x13: 'structure_type',
        0x14: 'subroutine', 0x15: 'subroutine_type', 0x16: 'typedef', 0x17: 'union_type',
        0x18: 'unspecified_parameters', 0x19: 'variant', 0x1a: 'common_block', 0x1b: 'common_inclusion',
        0x1c: 'inheritance', 0x1d: 'inlined_subroutine', 0x1e: 'module', 0x1f: 'ptr_to_member_type',
        0x20: 'set_type', 0x21: 'subrange_type', 0x22: 'with_stmt'}
AT_SIBLING, AT_LOCATION, AT_NAME, AT_STMT_LIST, AT_LOW_PC, AT_HIGH_PC, AT_LANGUAGE, AT_COMP_DIR, AT_PRODUCER = \
    0x0012, 0x0023, 0x0038, 0x0106, 0x0111, 0x0121, 0x0134, 0x01b8, 0x0258
LANG = {1: 'C89', 2: 'C', 4: 'C++', 0x8001: 'MW asm'}
OP_ADDR = 0x03


def parse_attrs(buf, o, end):
    attrs = {}
    while o < end:
        at = struct.unpack_from('>H', buf, o)[0]
        if at == 0:  # stripped DIE: its length swallows its children, whose headers start here
            break
        o += 2
        form = at & 0xf
        if form in (1, 2, 6):
            v = struct.unpack_from('>I', buf, o)[0]; o += 4
        elif form == 5:
            v = struct.unpack_from('>H', buf, o)[0]; o += 2
        elif form == 7:
            v = struct.unpack_from('>Q', buf, o)[0]; o += 8
        elif form == 3:
            n = struct.unpack_from('>H', buf, o)[0]; v = bytes(buf[o + 2:o + 2 + n]); o += 2 + n
        elif form == 4:
            n = struct.unpack_from('>I', buf, o)[0]; v = bytes(buf[o + 4:o + 4 + n]); o += 4 + n
        elif form == 8:
            e = buf.index(0, o); v = bytes(buf[o:e]).decode('latin1'); o = e + 1
        else:
            raise ValueError(f"unknown form {form} (attr {at:#06x}) at {o - 2:#x}")
        attrs[at] = v
    return attrs


def location_addr(block):
    """Static address from a location expression of the form OP_ADDR <addr>."""
    if block and len(block) >= 5 and block[0] == OP_ADDR:
        return struct.unpack_from('>I', block, 1)[0]
    return None


def main(elf_path, out_dir, splits_path=None):
    e = Elf(elf_path)
    # Section contents are already linked; the leftover .rela.debug only disagrees on a few lexical-block
    # high_pc values (linker-final values win), so use the raw bytes.
    dbg = e.data('.debug')
    line = e.data('.line')

    cus, funcs, vars_ = [], [], []
    tagcount = collections.Counter()
    cu = None
    o = 0
    depth_ends = []  # end offsets of open scopes, to know if a DIE is at CU level
    while o < len(dbg):
        ln = struct.unpack_from('>I', dbg, o)[0]
        if ln < 8:
            o += max(ln, 4)
            continue
        tag = struct.unpack_from('>H', dbg, o + 4)[0]
        attrs = parse_attrs(dbg, o + 6, o + ln)
        tagcount[TAGS.get(tag, hex(tag))] += 1
        if tag == 0x11:
            cu = dict(cu_index=len(cus), path=attrs.get(AT_NAME, ''), low_pc=attrs.get(AT_LOW_PC),
                      high_pc=attrs.get(AT_HIGH_PC), producer=attrs.get(AT_PRODUCER, ''),
                      language=LANG.get(attrs.get(AT_LANGUAGE), attrs.get(AT_LANGUAGE)),
                      comp_dir=attrs.get(AT_COMP_DIR, ''), stmt_list=attrs.get(AT_STMT_LIST),
                      debug_offset=o, functions=0, functions_stripped=0, variables=0)
            cus.append(cu)
        elif tag in (0x06, 0x14) or (tag == 0 and AT_HIGH_PC in attrs):
            lo, hi = attrs.get(AT_LOW_PC), attrs.get(AT_HIGH_PC)
            stripped = tag == 0 or not lo
            funcs.append(dict(name=attrs.get(AT_NAME, ''), address=lo, end=hi,
                              scope='global' if tag == 0x06 else ('static' if tag == 0x14 else 'unknown'),
                              stripped=stripped, cu=cu['cu_index'] if cu else None, debug_offset=o))
            if cu:
                cu['functions_stripped' if stripped else 'functions'] += 1
        elif tag in (0x07, 0x0c) or (tag == 0 and AT_LOCATION in attrs):
            addr = location_addr(attrs.get(AT_LOCATION))
            if addr is not None or tag == 0x07:  # skip stack/register locals
                vars_.append(dict(name=attrs.get(AT_NAME, ''), address=addr,
                                  scope='global' if tag == 0x07 else ('static' if tag == 0x0c else 'unknown'),
                                  stripped=tag == 0 or not addr, cu=cu['cu_index'] if cu else None,
                                  debug_offset=o))
                if cu:
                    cu['variables'] += 1
        o += ln
    print("DIE tags:", dict(tagcount.most_common(14)))

    # .line: per CU -> [u32 length][u32 base addr] then 10-byte rows {u32 line, u16 col, u32 addr delta}
    cu_lines = {}
    for c in cus:
        so = c['stmt_list']
        if so is None or so + 8 > len(line):
            continue
        length, base = struct.unpack_from('>II', line, so)
        rows = []
        for r in range(so + 8, so + length, 10):
            ln_, col, delta = struct.unpack_from('>IHI', line, r)
            rows.append((base + delta, ln_))
        rows.sort()
        cu_lines[c['cu_index']] = rows
    for f in funcs:
        rows = cu_lines.get(f['cu'])
        f['line'] = f['end_line'] = ''
        if rows and f['address'] and not f['stripped']:
            addrs = [a for a, _ in rows]
            i = bisect.bisect_left(addrs, f['address'])
            j = bisect.bisect_left(addrs, f['end'] or f['address'])
            span = [l for _, l in rows[i:j]] or ([rows[i][1]] if i < len(rows) else [])
            if span:
                f['line'], f['end_line'] = min(span), max(span)

    def base(p):
        return p.replace('/', '\\').rsplit('\\', 1)[-1]

    # Address -> CU lookups from debug info (live entries only)
    func_at = {f['address']: f for f in funcs if not f['stripped']}

    # CodeWarrior emits a global_variable entry with a location in every CU that *references* a
    # global, not just the one defining it. Resolve the definer with the project's split map
    # (which unit's data range holds the address); unresolved multi-CU variables stay ambiguous.
    unit_ranges = load_splits(splits_path) if splits_path else []
    unit_starts = [r[0] for r in unit_ranges]

    def unit_for(addr):
        i = bisect.bisect_right(unit_starts, addr) - 1
        if i >= 0 and unit_ranges[i][0] <= addr < unit_ranges[i][1]:
            return unit_ranges[i][2]
        return None

    mentions = collections.defaultdict(list)
    for v in vars_:
        if not v['stripped'] and v['cu'] is not None:
            mentions[v['address']].append(v)
    var_at, ambiguous = {}, set()
    for addr, vs in mentions.items():
        cands = {cus[v['cu']]['cu_index'] for v in vs}
        chosen = None
        if len(cands) == 1:
            chosen = vs[0]
        else:
            unit = unit_for(addr)
            match = [v for v in vs if unit and base(cus[v['cu']]['path']) == unit.rsplit('/', 1)[-1]]
            if match:
                chosen = match[0]
            else:
                ambiguous.add(addr)
        if chosen:
            var_at[addr] = chosen
        for v in vs:
            v['defined_here'] = '' if chosen is None else int(v is chosen or v['cu'] == chosen['cu'])
    print(f"variables mentioned by several CUs: {sum(1 for vs in mentions.values() if len({v['cu'] for v in vs}) > 1)}"
          f", resolved via splits: {sum(1 for a, vs in mentions.items() if len({v['cu'] for v in vs}) > 1 and a in var_at)}"
          f", still ambiguous: {len(ambiguous)}")
    ranges = sorted((c['low_pc'], c['high_pc'], c['cu_index']) for c in cus if c['low_pc'] and c['high_pc'])

    def cu_for_text(addr):
        i = bisect.bisect_right([r[0] for r in ranges], addr) - 1
        if i >= 0 and ranges[i][0] <= addr < ranges[i][1]:
            return ranges[i][2]
        return None

    # Rewrite the symbol CSV with source columns
    secname = lambda n: {0: 'UND', 0xfff1: 'ABS', 0xfff2: 'COMMON'}.get(n, e.secs[n]['name'] if n < len(e.secs) else str(n))
    TY = {0: 'NOTYPE', 1: 'OBJECT', 2: 'FUNC', 3: 'SECTION', 4: 'FILE'}
    BI = {0: 'LOCAL', 1: 'GLOBAL', 2: 'WEAK'}
    rows, cur_file = [], ''
    how = collections.Counter()
    for s in e.syms[1:]:
        if s['type'] == 4:
            cur_file = s['name']
        src_path, src, line_no, via = '', '', '', ''
        if s['type'] in (2, 1, 0):
            dbg_ent = func_at.get(s['value']) if s['type'] == 2 else var_at.get(s['value'])
            if dbg_ent is None and s['type'] == 0:
                dbg_ent = func_at.get(s['value']) or var_at.get(s['value'])
            if dbg_ent is not None and dbg_ent['cu'] is not None:
                src_path = cus[dbg_ent['cu']]['path']; via = 'dwarf'
                line_no = dbg_ent.get('line', '')
            elif s['type'] != 2 and s['value'] in ambiguous:
                via = 'dwarf_ambiguous'
            elif s['type'] == 2 and s['value'] and cu_for_text(s['value']) is not None:
                src_path = cus[cu_for_text(s['value'])]['path']; via = 'dwarf_cu_range'
            if s['bind'] == 0 and cur_file:
                if not src_path:
                    via = 'symtab_file'
                src = cur_file if not src_path else base(src_path)
            elif src_path:
                src = base(src_path)
        rows.append(dict(index=s['index'], name=s['name'], address=f"0x{s['value']:08X}", size=s['size'],
                         type=TY.get(s['type'], s['type']), bind=BI.get(s['bind'], s['bind']), section=secname(s['shndx']),
                         source_file=src, source_path=src_path, line=line_no, source_via=via))

    # The linker lays out each object file contiguously per section, so an unattributed symbol whose
    # nearest attributed neighbours on both sides (same section) share a file is inferred to be from it.
    by_sec = collections.defaultdict(list)
    for r in rows:
        if r['type'] in ('FUNC', 'OBJECT') and r['address'] != '0x00000000':
            by_sec[r['section']].append(r)
    for lst in by_sec.values():
        lst.sort(key=lambda r: (r['address'], r['source_file'] == ''))
        known = [i for i, r in enumerate(lst) if r['source_file']]
        for a, b in zip(known, known[1:]):
            if b - a > 1 and lst[a]['source_file'] == lst[b]['source_file']:
                for r in lst[a + 1:b]:
                    if not r['source_file']:
                        r['source_file'], r['source_path'] = lst[a]['source_file'], lst[a]['source_path']
                        r['source_via'] = 'address_neighbors'
    for r in rows:
        if r['type'] in ('FUNC', 'OBJECT'):
            how[(r['type'], r['source_via'] or 'none')] += 1
    write(os.path.join(out_dir, 'rush_d_symbols.csv'), rows)
    print("symbol source attribution:", dict(sorted(how.items())))

    write(os.path.join(out_dir, 'rush_d_compile_units.csv'),
          [dict(cu_index=c['cu_index'], source_file=base(c['path']), source_path=c['path'],
                low_pc=hx(c['low_pc']), high_pc=hx(c['high_pc']),
                code_bytes=(c['high_pc'] - c['low_pc']) if c['low_pc'] and c['high_pc'] else 0,
                functions=c['functions'], functions_stripped=c['functions_stripped'], variables=c['variables'],
                language=c['language'], producer=c['producer'], debug_offset=hx(c['debug_offset'])) for c in cus])
    write(os.path.join(out_dir, 'rush_d_functions.csv'),
          [dict(name=f['name'], address=hx(f['address']), end=hx(f['end']),
                size=(f['end'] - f['address']) if f['address'] and f['end'] and not f['stripped'] else '',
                scope=f['scope'], stripped=int(f['stripped']), source_file=base(cus[f['cu']]['path']) if f['cu'] is not None else '',
                line=f['line'], end_line=f['end_line'], source_path=cus[f['cu']]['path'] if f['cu'] is not None else '',
                debug_offset=hx(f['debug_offset'])) for f in funcs])
    write(os.path.join(out_dir, 'rush_d_variables.csv'),
          [dict(name=v['name'], address=hx(v['address']), scope=v['scope'], stripped=int(v['stripped']),
                defined_here=v.get('defined_here', ''),
                source_file=base(cus[v['cu']]['path']) if v['cu'] is not None else '',
                source_path=cus[v['cu']]['path'] if v['cu'] is not None else '',
                debug_offset=hx(v['debug_offset'])) for v in vars_])
    live = [f for f in funcs if not f['stripped']]
    print(f"compile units: {len(cus)}  functions: {len(funcs)} ({len(live)} live, {len(funcs) - len(live)} stripped)"
          f"  static-address variables: {len(vars_)}  CUs with line tables: {len(cu_lines)}")
    print("producers:", dict(collections.Counter(c['producer'] for c in cus).most_common()))
    print("languages:", dict(collections.Counter(c['language'] for c in cus).most_common()))


def hx(v):
    return f"0x{v:08X}" if isinstance(v, int) else ''


def load_splits(path):
    """(start, end, unit) for every section range in a dtk splits.txt, sorted by start."""
    out, unit, header = [], None, True
    for ln in open(path, encoding='utf-8').read().split('\n'):
        if header:
            header = ln != ''
            continue
        if ln and not ln[0].isspace() and ln.endswith(':'):
            unit = ln[:-1]
            continue
        parts = dict(p.split(':', 1) for p in ln.split()[1:] if ':' in p)
        if 'start' in parts and 'end' in parts:
            out.append((int(parts['start'], 16), int(parts['end'], 16), unit))
    return sorted(out)


def write(path, rows):
    with open(path, 'w', newline='', encoding='utf-8') as o:
        w = csv.DictWriter(o, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    print(f"wrote {path} ({len(rows):,} rows)")


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else None)
