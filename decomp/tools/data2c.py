"""Emit C definitions for data objects in a unit's split assembly.

usage: python tools/data2c.py <unit> [symbol ...]
Types come from rush_d_dwarf.c. Handles scalars and (multi-dimensional) arrays of
integers, floats, doubles, chars and pointers; anything else is printed as a raw
byte comment to fill in by hand. Section placement implies qualifiers: .rodata and
.sdata2 objects are emitted `const`.
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ctx import asm_text, dwarf_section  # noqa: E402

DECL = re.compile(r'^(?P<static>static )?(?P<type>[A-Za-z_][\w \*]*?)\s*(?P<name>\w+)(?P<dims>(?:\[\d+\])*); // size: (?P<size>0x[0-9A-F]+), address: (?P<addr>0x[0-9A-F]+)', re.M)
SCALARS = {
    'unsigned char': ('B', 1), 'signed char': ('b', 1), 'char': ('b', 1),
    'unsigned short': ('H', 2), 'signed short': ('h', 2), 'short': ('h', 2),
    'unsigned int': ('I', 4), 'int': ('i', 4), 'unsigned long': ('I', 4), 'long': ('i', 4),
    'float': ('f', 4), 'double': ('d', 8),
}


def parse_asm(text):
    """name -> (section, bytes, {offset: reloc symbol})"""
    objs, sec, cur = {}, None, None
    for ln in text.splitlines():
        s = ln.strip()
        m = re.match(r'^\.section (\.\w+)|^\.(rodata|data|bss|text)$', s)
        if m:
            sec = m.group(1) or '.' + m.group(2)
            continue
        m = re.match(r'^\.obj (\S+), \w+', s)
        if m:
            cur = [sec, bytearray(), {}]
            objs[m.group(1).strip('"')] = cur
            continue
        if s.startswith('.endobj'):
            cur = None
            continue
        if cur is None:
            continue
        m = re.match(r'^\.(4byte|2byte|byte|float|double|skip|string|asciz)\s+(.*)$', s)
        if not m:
            continue
        kind, arg = m.groups()
        if kind == '4byte':
            if re.match(r'^0x[0-9A-Fa-f]+$|^-?\d+$', arg):
                cur[1] += struct.pack('>I', int(arg, 0) & 0xffffffff)
            else:
                cur[2][len(cur[1])] = arg.strip('"')
                cur[1] += b'\0\0\0\0'
        elif kind == '2byte':
            for a in arg.split(','):
                cur[1] += struct.pack('>H', int(a.strip(), 0) & 0xffff)
        elif kind == 'byte':
            for a in arg.split(','):
                cur[1] += bytes([int(a.strip(), 0) & 0xff])
        elif kind == 'float':
            cur[1] += struct.pack('>f', float(arg))
        elif kind == 'double':
            cur[1] += struct.pack('>d', float(arg))
        elif kind == 'skip':
            cur[1] += bytes(int(arg, 0))
        elif kind in ('string', 'asciz'):
            cur[1] += eval(arg).encode('latin1') + b'\0'
    return objs


def float_lit(v, double=False):
    if v == int(v) and abs(v) < 1e9 and not (v == 0 and str(v).startswith('-')):
        return f"{int(v)}.0" + ('' if double else 'f')
    for p in range(1, 18):
        s = f"{v:.{p}g}"
        if (struct.unpack('>d', struct.pack('>d', float(s)))[0] if double else
                struct.unpack('>f', struct.pack('>f', float(s)))[0]) == v:
            break
    if 'e' not in s and '.' not in s and 'inf' not in s and 'nan' not in s:
        s += '.0'
    return s if double else s + 'f'


def elems(data, relocs, fmt, size):
    out = []
    for i in range(0, len(data), size):
        if i in relocs:
            out.append('&' + relocs[i] if not relocs[i].startswith('"') else relocs[i])
            continue
        v = struct.unpack('>' + fmt, data[i:i + size])[0]
        if fmt == 'f':
            out.append(float_lit(v))
        elif fmt == 'd':
            out.append(float_lit(v, True))
        elif fmt in 'IH' and v > 9:
            out.append(hex(v))
        else:
            out.append(str(v))
    return out


def nest(items, dims):
    if len(dims) <= 1:
        return items
    step = len(items) // dims[0]
    return ['{ ' + ', '.join(nest(items[i:i + step], dims[1:])) + ' }' for i in range(0, len(items), step)]


def emit(decl, obj):
    sec, data, relocs = obj
    t, name, dims = decl['type'].strip(), decl['name'], [int(x) for x in re.findall(r'\[(\d+)\]', decl['dims'])]
    const = 'const ' if sec in ('.rodata', '.sdata2') else ''
    static = 'static ' if decl['static'] else ''
    size = int(decl['size'], 16)
    data = data[:size]
    if t.endswith('*'):
        fmt, esz = 'I', 4
    elif t in SCALARS:
        fmt, esz = SCALARS[t]
    else:
        return f"/* {name}: type '{t}' needs a hand-written initializer; {sec}, {size:#x} bytes:\n   {data.hex()} relocs={relocs} */"
    if sec in ('.bss', '.sbss'):
        return f"{static}{t} {name}{decl['dims']};"
    vals = elems(data, relocs, fmt, esz)
    if t == 'char' and dims and all(v.lstrip('-').isdigit() for v in vals):
        raw = bytes(int(v) & 0xff for v in vals)
        if raw.rstrip(b'\0') and all(32 <= c < 127 for c in raw.rstrip(b'\0')):
            return f'{static}{const}{t} {name}{decl["dims"]} = "{raw.rstrip(b"\\0").decode()}";'
    if not dims:
        return f"{static}{const}{t} {name} = {vals[0]};"
    body = nest(vals, dims)
    lines, cur = [], '    '
    for v in body:
        piece = v + ', '
        if len(cur) + len(piece) > 100:
            lines.append(cur.rstrip())
            cur = '    '
        cur += piece
    lines.append(cur.rstrip().rstrip(','))
    return f"{static}{const}{t} {name}{decl['dims']} = {{\n" + '\n'.join(lines) + "\n};"


def main():
    unit, wanted = sys.argv[1], sys.argv[2:]
    objs = parse_asm(asm_text(unit))
    decls = {m['name']: m for m in DECL.finditer(dwarf_section(unit) or '')}
    names = wanted or sorted(objs, key=lambda n: (objs[n][0], 0))
    for n in names:
        if n.startswith(('gap_', '@')) and n not in wanted:
            continue
        if n not in objs:
            print(f"/* {n}: not in asm */")
        elif n not in decls:
            print(f"/* {n}: no DWARF declaration; {objs[n][0]} {len(objs[n][1]):#x} bytes */")
        else:
            print(emit(decls[n], objs[n]))
        print()


if __name__ == '__main__':
    main()
