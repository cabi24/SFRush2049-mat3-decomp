"""Generate C data definitions for a unit from its DWARF section + split assembly.

usage: python tools/unit2c.py <unit> [--only name,name] [--no-types]

Type statements (struct/union/enum/typedef) are copied from rush_d_dwarf.c. Every
variable with data gets an initializer built from the original bytes, using the
member layout parsed from the same text (brace elision is used, so nested members
are a flat list). Pointer slots use the relocation target: string-literal labels
(@NNN with string data) become "..." literals, other targets become &symbol, with an
extern declaration emitted for symbols this unit doesn't define.
Ordering: initialised objects in address order; .bss/.sbss in reverse address order
(CodeWarrior lays out uninitialised data in reverse declaration order).
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ctx import asm_text, dwarf_section  # noqa: E402
from data2c import parse_asm, float_lit  # noqa: E402

SCALAR = {
    'unsigned char': ('B', 1), 'signed char': ('b', 1), 'char': ('b', 1), 'bool': ('B', 1),
    'unsigned short': ('H', 2), 'signed short': ('h', 2), 'short': ('h', 2), 'wchar_t': ('H', 2),
    'unsigned int': ('I', 4), 'int': ('i', 4), 'signed int': ('i', 4), 'unsigned long': ('I', 4), 'long': ('i', 4),
    'unsigned long long': ('Q', 8), 'long long': ('q', 8), 'signed long long': ('q', 8),
    'float': ('f', 4), 'double': ('d', 8), 'long double': ('d', 8),
}


def statements(text):
    """Split DWARF C text into top-level statements: (kind, text)."""
    out, buf, depth = [], [], 0
    lines = text.split('\n')
    i = 0
    while i < len(lines):
        ln = lines[i]
        if depth == 0 and (ln.startswith('// Range:') or ln.startswith('// Erased')):
            # function: skip header + body
            j = i + 1
            while j < len(lines) and '{' not in lines[j]:
                j += 1
            d = 0
            while j < len(lines):
                d += lines[j].count('{') - lines[j].count('}')
                if d <= 0:
                    break
                j += 1
            i = j + 1
            buf = []
            continue
        if depth == 0 and (ln.startswith('/*') or not ln.strip()):
            if ln.startswith('/*'):
                while i < len(lines) and '*/' not in lines[i]:
                    i += 1
            i += 1
            continue
        buf.append(ln)
        depth += ln.count('{') - ln.count('}')
        if depth == 0 and ln.rstrip().split('//')[0].rstrip().endswith(';'):
            out.append('\n'.join(buf))
            buf = []
        i += 1
    return out


VAR_TAIL = re.compile(r'\s*(?P<name>\w+)(?P<dims>(?:\[\d+\])*);\s*// size: (?P<size>0x[0-9A-F]+), address: (?P<addr>0x[0-9A-F]+)\s*$')


def split_var(stmt):
    """(type_text, name, dims, size, addr) for a variable statement, else None."""
    last = stmt.rstrip().split('\n')[-1]
    m = re.search(r'(?P<pre>.*?)(?P<name>\w+)(?P<dims>(?:\[\d+\])*);\s*// size: (?P<size>0x[0-9A-F]+), address: (?P<addr>0x[0-9A-F]+)\s*$', last)
    if not m:
        return None
    head = stmt.rstrip().split('\n')[:-1] + [m.group('pre')]
    type_text = '\n'.join(head)
    return type_text, m.group('name'), m.group('dims'), int(m.group('size'), 16), int(m.group('addr'), 16)


def dims_of(s):
    return [int(x) for x in re.findall(r'\[(\d+)\]', s)]


class Layout:
    def __init__(self, named):
        self.named = named  # 'struct _tCar' -> member text

    def slots(self, type_text, dims=(), base=0):
        """Leaf slots [(offset, kind)] where kind is a struct format char or 'p' (pointer)."""
        if '{' not in type_text:
            type_text = re.sub(r'//[^\n]*', '', type_text)
        else:  # keep member '// offset' comments; drop leading '// total size' lines
            type_text = re.sub(r'^(\s*//[^\n]*\n)+', '', type_text)
        type_text = re.sub(r'/\*.*?\*/', '', type_text, flags=re.S).strip()
        type_text = re.sub(r'^(static|const|volatile)\s+', '', type_text)
        elem = self._elem(type_text)
        esize = elem[1]
        out = []
        count = 1
        for d in dims:
            count *= d
        for k in range(count):
            for off, kind in elem[0]:
                out.append((base + k * esize + off, kind))
        return out

    def _elem(self, t):
        """([(off, kind)], size) for one element of type text t."""
        t = t.strip()
        if '{' in t:
            hdr, body = t.split('{', 1)
            body = body.rsplit('}', 1)
            after = body[1] if len(body) > 1 else ''
            body = body[0]
            if '*' in after:
                return [(0, 'p')], 4
            hdr = hdr.strip()
            if hdr.startswith('enum'):
                return [(0, 'i')], 4
            return self._members(body, union=hdr.startswith('union'))
        if '*' in t or '(' in t:
            return [(0, 'p')], 4
        t = re.sub(r'\s+', ' ', t)
        if t.startswith('enum'):
            return [(0, 'i')], 4
        if t in SCALAR:
            return [(0, SCALAR[t][0])], SCALAR[t][1]
        if t in self.named:
            return self._members(self.named[t], union=t.startswith('union'))
        raise KeyError(t)

    def _members(self, body, union=False):
        """Parse member list text; returns (slots, size)."""
        slots, depth, buf = [], 0, []
        size = 0
        for ln in body.split('\n'):
            if depth == 0 and ln.strip().startswith('// total size'):
                continue
            buf.append(ln)
            depth += ln.count('{') - ln.count('}')
            if depth == 0:
                fp = re.search(r'\(\*\s*(\w+)\)\s*\(.*\)\s*;\s*// offset (0x[0-9A-F]+), size (0x[0-9A-F]+)', ln)
                if fp:  # function-pointer member: one pointer slot
                    off, msize = int(fp.group(2), 16), int(fp.group(3), 16)
                    if not union or not slots:
                        slots.append((off, 'p'))
                    size = max(size, off + msize)
                    buf = []
                    continue
                m = re.search(r'(\w+)((?:\[\d+\])*);\s*// offset (0x[0-9A-F]+), size (0x[0-9A-F]+)', ln)
                if m:
                    text = '\n'.join(buf)
                    pre = text[:text.rfind(m.group(1) + m.group(2) + ';')]
                    off = int(m.group(3), 16)
                    msize = int(m.group(4), 16)
                    sub = self.slots(pre, dims_of(m.group(2)), off)
                    if not union or not slots:
                        slots.extend(sub)
                    size = max(size, off + msize)
                    buf = []
                elif ln.strip() == '' or ln.strip().startswith('//'):
                    buf = []
        return slots, size


def value(data, relocs, off, kind, strings, externs, local_names):
    if kind == 'p':
        tgt = relocs.get(off)
        if tgt is None:
            v = struct.unpack_from('>I', data, off)[0]
            return '0' if v == 0 else f'(void *){v:#x}'
        if tgt in strings:
            return c_string(strings[tgt])
        if tgt not in local_names:
            externs.add(tgt)
        return '(void *)&' + tgt
    fmt = kind
    size = struct.calcsize('>' + fmt)
    v = struct.unpack_from('>' + fmt, data, off)[0]
    if fmt == 'f':
        return float_lit(v)
    if fmt == 'd':
        return float_lit(v, True)
    if fmt in 'IHQ' and v > 9:
        return hex(v)
    return str(v)


def c_string(b):
    s = b.rstrip(b'\0').decode('latin1')
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n').replace('\t', '\\t') + '"'


def main():
    unit = sys.argv[1]
    only = None
    if '--only' in sys.argv:
        only = set(sys.argv[sys.argv.index('--only') + 1].split(','))
    no_types = '--no-types' in sys.argv
    text = dwarf_section(unit)
    objs = parse_asm(asm_text(unit))
    strings = {n: o[1] for n, o in objs.items() if n.startswith('@') and o[1] and o[1].endswith(b'\0')
               and all(32 <= c < 127 or c in (9, 10) for c in o[1].rstrip(b'\0'))}
    stmts = statements(text)
    named = {}
    types, vars_ = [], []
    for st in stmts:
        v = split_var(st)
        if v:
            vars_.append((st, v))
            continue
        m = re.match(r'^\s*(?://[^\n]*\n)*\s*((?:struct|union|enum) \w+) \{', st)
        if m:
            named[m.group(1)] = st.split('{', 1)[1].rsplit('}', 1)[0]
        types.append(st)
    lay = Layout(named)
    local_names = {v[1] for _, v in vars_}
    externs = set()
    defs = []
    for st, (ttext, name, dims, size, addr) in vars_:
        if only and name not in only:
            continue
        obj = objs.get(name)
        decl = st.rstrip()
        decl = re.sub(r';\s*// size: .*$', '', decl)
        if obj is None:
            defs.append((addr, 'extern', f"/* {name}: not in this unit's asm; declared extern */\nextern {decl};"))
            continue
        sec, data, relocs = obj
        if sec in ('.bss', '.sbss'):
            defs.append((addr, sec, decl + ';'))
            continue
        try:
            slots = lay.slots(ttext, dims_of(dims))
            vals = [value(data, relocs, off, kind, strings, externs, local_names) for off, kind in slots]
        except (KeyError, struct.error) as ex:
            defs.append((addr, sec, f"/* {name}: could not lay out ({ex}); {sec} {size:#x} bytes: {bytes(data).hex()} */"))
            continue
        const = 'const ' if sec in ('.rodata', '.sdata2') and not decl.startswith('const') else ''
        if const and decl.lstrip().startswith('//'):  # put the qualifier after leading comment lines
            lead = re.match(r'^((?:\s*//[^\n]*\n)+)', decl).group(1)
            decl, const = lead + const + decl[len(lead):].lstrip(), ''
        body = ', '.join(vals)
        if len(body) > 90:
            chunks, cur = [], ''
            for v in vals:
                if len(cur) + len(v) + 2 > 96:
                    chunks.append(cur.rstrip())
                    cur = ''
                cur += v + ', '
            chunks.append(cur.rstrip().rstrip(','))
            body = '\n    ' + '\n    '.join(chunks) + '\n'
        defs.append((addr, sec, f"{const}{decl} = {{ {body} }};" if '\n' not in body else f"{const}{decl} = {{{body}}};"))
    if not no_types:
        for t in types:
            print(t.strip())
            print()
    for e in sorted(externs - local_names):
        print(f"extern char {e}[];")
    if externs - local_names:
        print()
    init = sorted([d for d in defs if d[1] not in ('.bss', '.sbss')], key=lambda d: d[0])
    unin = sorted([d for d in defs if d[1] in ('.bss', '.sbss')], key=lambda d: -d[0])
    for _, _, d in init + unin:
        print(d)
        print()


if __name__ == '__main__':
    main()
