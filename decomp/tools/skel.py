"""Generate a C skeleton for a unit: shared header, data with initializers, function stubs.

usage: python tools/skel.py <unit> [--data-only] > src/<unit>.c

Uses include/rush/rush_decls.json (from tools/dwarftypes.py) for declarations in the shared
type names, and the unit2c layout logic for initializer values from the original bytes.
Functions come in address order with signature, locals (register noted) and a TODO body;
the original assembly is in build/2049LE/asm/<unit>.s. Compiler-generated objects (@NNN
literals, gap_ padding) are not emitted: they come from the code once it is written.
Prototype overrides take precedence. Addressed DWARF definitions and dtk object
names recover local statics and initialized mid-file globals; references do not.
Nested local scopes lost by rush_decls.json remain explicit TODO comments.
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ctx import asm_text, dwarf_section  # noqa: E402
from data2c import parse_asm  # noqa: E402
import unit2c  # noqa: E402

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
BS = chr(92)


def object_addresses(assembly):
    """dtk object names -> original addresses (not section-relative offsets)."""
    return {m.group(2).strip('"'): int(m.group(1), 16) for m in re.finditer(
        r'^# \.\w+:[^\n]*\|\s*(0x[0-9A-Fa-f]+)\s*\|[^\n]*\n'
        r'\.obj ([^,\n]+),', assembly, re.M)}


def object_symbol(name, addr, objs, addresses):
    """Resolve a definition by address; never guess among repeated local names."""
    matches = [n for n, a in addresses.items() if a == addr and
               (n == name or n.startswith(name + '$')) and n in objs]
    if len(matches) == 1:
        return matches[0]
    if not matches and name in objs and name not in addresses:
        return name  # older assembly dumps without address comments
    return None


def definition_sites(text, functions):
    """(name, address) -> (following function, after first function).

    Only top-level DWARF definitions have a size/address trailer. `// ->`
    references inside functions must never establish ownership or placement.
    Local statics are printed before their owner in this DWARF dump; the
    assembly's $ suffix distinguishes them from mid-file file-scope data.
    """
    markers = []
    starts = list(re.finditer(r'^// (?:Range:|Erased)[^\n]*\n', text, re.M))
    for i, m in enumerate(starts):
        end = starts[i + 1].start() if i + 1 < len(starts) else len(text)
        # Anonymous return aggregates can span many lines before `} *f(...)`.
        # Ignore reference comments, then select the first known function name.
        header = re.sub(r'//[^\n]*', '', text[m.end():end])
        names = [(match.start(), f['name']) for f in functions
                 if (match := re.search(r'\b' + re.escape(f['name']) + r'\s*\(', header))]
        markers.append((m.start(), min(names)[1] if names else None))
    sites = {}
    offset = 0
    for st in unit2c.statements(text):
        pos = text.find(st, offset)
        if pos < 0:
            continue
        offset = pos + len(st)
        v = unit2c.split_var(st)
        if v:
            following = next((name for p, name in markers if p > pos), None)
            sites[(v[1], v[4])] = (following, bool(markers and pos > markers[0][0]))
    return sites


def declaration_name(decl):
    """Identifier in the scalar/array/pointer declarations emitted by dwarftypes."""
    m = re.search(r'\(\s*\*\s*(\w+)\s*(?:\[[^]]*\]\s*)*\)', decl)
    if not m:
        m = re.search(r'\b(\w+)\s*(?:\[[^]]*\]\s*)*$', decl.strip())
    return m.group(1) if m else None


def cu_for(unit, decls):
    want = unit.replace('/', BS)
    for path, cu in decls.items():
        if path.rsplit('.', 1)[0].endswith(BS + want):
            return cu
    return None


def initializer_values(unit):
    """name -> list of C value strings, using unit2c's layout of the dtk text."""
    text = dwarf_section(unit) or ''
    assembly = asm_text(unit)
    objs = parse_asm(assembly)
    addresses = object_addresses(assembly)
    strings = {n: o[1] for n, o in objs.items() if n.startswith('@') and o[1] and o[1].endswith(b'\0')
               and all(32 <= c < 127 or c in (9, 10) for c in o[1].rstrip(b'\0'))}
    named, vars_ = {}, []
    for st in unit2c.statements(text):
        v = unit2c.split_var(st)
        if v:
            vars_.append(v)
            continue
        m = re.match(r'^\s*(?://[^\n]*\n)*\s*((?:struct|union|enum) \w+) \{', st)
        if m:
            named[m.group(1)] = st.split('{', 1)[1].rsplit('}', 1)[0]
    lay = unit2c.Layout(named)
    local = {v[1] for v in vars_}
    out, externs = {}, set()
    for ttext, name, dims, size, addr in vars_:
        symbol = object_symbol(name, addr, objs, addresses)
        obj = objs.get(symbol)
        if not obj or obj[0] in ('.bss', '.sbss'):
            continue
        try:
            slots = lay.slots(ttext, unit2c.dims_of(dims))
            out[symbol] = [unit2c.value(obj[1], obj[2], off, kind, strings, externs, local) for off, kind in slots]
        except (KeyError, Exception) as ex:  # noqa: BLE001
            out[symbol] = None
    return out, objs


def wrap(vals):
    body, cur = [], ''
    for v in vals:
        if len(cur) + len(v) + 2 > 96:
            body.append(cur.rstrip())
            cur = ''
        cur += v + ', '
    body.append(cur.rstrip().rstrip(','))
    return '{\n    ' + '\n    '.join(body) + '\n}' if len(body) > 1 or len(body[0]) > 80 else '{ ' + body[0] + ' }'


def data_definition(v, symbol, values, objs):
    """Use the same initializer/section rules for file data and local statics."""
    sec = objs[symbol][0]
    static = 'static ' if v['static'] else ''
    const = 'const ' if sec in ('.rodata', '.sdata2') and not v['decl'].startswith('const ') else ''
    if sec in ('.bss', '.sbss'):
        return f"{static}{v['decl']};"
    if values.get(symbol) is None:
        return f"/* TODO initializer: {static}{const}{v['decl']} ({sec}) */"
    scalar = len(values[symbol]) == 1 and '[' not in v['decl'] and v['decl'].split()[0] in (
        'float', 'double', 'int', 'unsigned', 'signed', 'char', 'short', 'long', 'void')
    vals = values[symbol]
    if re.match(r'^(const )?unsigned char \*', v['decl']):
        vals = ['(unsigned char *)' + x if x.startswith('"') else x for x in vals]
    if not scalar and all(x in ('0', '0.0f', '0.0') for x in vals):
        init_txt = '{ 0 }'
    else:
        init_txt = vals[0] if scalar else wrap(vals)
    return f"{static}{const}{v['decl']} = {init_txt};"


def emit(unit, cu, values, objs, assembly, dwarf, overrides=None, data_only=False):
    overrides = overrides or {}
    addresses = object_addresses(assembly)
    sites = definition_sites(dwarf, cu['functions'])
    local_defs, local_names, mid_defs, unresolved = {}, {}, {}, []
    # Stripped function bodies are not emitted. Keep file data at the next live
    # boundary instead of silently losing it with the stripped owner's marker.
    anchors = {f['name']: next((g['name'] for g in cu['functions'][i:]
                               if not g['stripped']), None)
               for i, f in enumerate(cu['functions'])}
    print(f"/* {unit.rsplit('/', 1)[-1]}.c - TODO: describe. Status: in progress.")
    print(f"   Original: {cu['path']}; assembly: build/2049LE/asm/{unit}.s */")
    print('#include "rush/rush_protos.h"')
    print()
    # data: definitions for variables placed in this unit
    defs = []
    for v in cu['variables']:
        name = v['name']
        if not v['addr']:
            continue
        symbol = object_symbol(name, v['addr'], objs, addresses)
        if symbol is None:
            continue
        sec = objs[symbol][0]
        d = (sec, v['addr'], data_definition(v, symbol, values, objs))
        owner, mid = sites.get((name, v['addr']), (None, False))
        if symbol.startswith(name + '$'):
            if owner is None or not v['static']:
                unresolved.append(f"/* TODO local-static scope: {symbol} at {v['addr']:#010x}; {v['decl']} */")
            else:
                local_defs.setdefault(owner, []).append(d)
                local_names.setdefault(owner, set()).add(name)
        elif mid and sec not in ('.bss', '.sbss') and not data_only:
            mid_defs.setdefault(anchors.get(owner), []).append(d)
        else:
            defs.append(d)
    init = sorted([d for d in defs if d[0] not in ('.bss', '.sbss')], key=lambda d: d[1])
    unin = sorted([d for d in defs if d[0] in ('.bss', '.sbss')], key=lambda d: -d[1])
    live = sorted([f for f in cu['functions'] if not f['stripped']], key=lambda f: f['lo'])
    stripped = [f['name'] for f in cu['functions'] if f['stripped']]
    statics = [f for f in live if f['static']]

    def signature(f):
        return re.sub(r'^static\s+', '', overrides.get('prototypes', {}).get(
            f['name'], f['decl']).rstrip().rstrip(';').rstrip())

    if not data_only:
        # prototypes first: data initialisers may take a static function's address
        for f in statics:
            print(f"static {signature(f)};")
        if statics:
            print()
    for _, _, d in init + unin:
        print(d)
    if init or unin:
        print()
    if data_only:
        return
    for d in unresolved:
        print(d)
    if stripped:
        print("/* Stripped by the linker (string literals or array initialisers they used can remain in")
        print(f"   .rodata/.sdata2; see writeup): {', '.join(stripped)} */")
        print()
    for f in live:
        for _, _, d in mid_defs.get(f['name'], []):
            print(d)
            print()
        print(f"/* {f['lo']:#010x}, size {f['hi'] - f['lo']:#x} */")
        print(('static ' if f['static'] else '') + signature(f))
        print('{')
        seen = {declaration_name(p['decl']) for p in f['params']}
        # Include named parameters added by a prototype override. Split only at
        # outer commas so pointer-to-array/function parameter types stay intact.
        params = re.search(r'\b' + re.escape(f['name']) + r'\s*\((.*)\)', signature(f))
        if params:
            depth, start = 0, 0
            for i, c in enumerate(params.group(1) + ','):
                depth += (c in '([') - (c in ')]')
                if c == ',' and depth == 0:
                    seen.add(declaration_name(params.group(1)[start:i]))
                    start = i + 1
        for _, _, d in local_defs.get(f['name'], []):
            print('    ' + d.replace('\n', '\n    '))
        seen.update(local_names.get(f['name'], ()))
        for l in f['locals']:
            name = declaration_name(l['decl'])
            if name and name in seen:
                print(f"    /* TODO restore DWARF block scope: {l['decl']}; ({l['reg'] or 'stack'}) */")
                continue
            seen.add(name)
            if l['static_addr']:
                symbol = object_symbol(name, l['static_addr'], objs, addresses)
                if symbol:
                    d = data_definition({'decl': l['decl'], 'static': True}, symbol, values, objs)
                    print('    ' + d.replace('\n', '\n    '))
                else:
                    print(f"    /* static local at {l['static_addr']:#010x}: static {l['decl']}; */")
            else:
                print(f"    {l['decl']}; /* {l['reg'] or 'stack'} */")
        print('    /* TODO */')
        print('}')
        print()
    for _, _, d in mid_defs.get(None, []):
        print(d)
        print()
    for owner in sorted(local_defs.keys() - {f['name'] for f in live}):
        print(f"/* TODO: local statics of stripped {owner} require a placeholder body. */")
        for _, _, d in local_defs[owner]:
            print(d if d.startswith('/*') else '/* ' + d + ' */')


def main():
    unit = sys.argv[1]
    data_only = '--data-only' in sys.argv
    with open(os.path.join(ROOT, 'include', 'rush', 'rush_decls.json')) as stream:
        decls = json.load(stream)
    cu = cu_for(unit, decls)
    if cu is None:
        sys.exit(f"no compile unit for {unit}")
    ov_path = os.path.join(ROOT, 'include', 'rush', 'overrides.json')
    overrides = {}
    if os.path.exists(ov_path):
        with open(ov_path) as stream:
            overrides = json.load(stream)
    values, objs = initializer_values(unit)
    emit(unit, cu, values, objs, asm_text(unit), dwarf_section(unit) or '', overrides, data_only)


if __name__ == '__main__':
    main()
