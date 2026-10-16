"""Print the reconstruction context for one unit: its DWARF section and its split assembly.

usage: python tools/ctx.py GameSource/src/timer [--dwarf-only | --asm-only]
"""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
DWARF = os.path.join(ROOT, '..', 'rush_d_dwarf.c')
BS = chr(92)


def dwarf_section(unit):
    """Text of rush_d_dwarf.c belonging to the compile unit whose path ends with unit + '.c/.cpp'."""
    want = unit.replace('/', BS)
    text = open(DWARF, encoding='utf-8', errors='replace').read()
    starts = [m.start() for m in re.finditer(r'/\*\n    Compile unit: ', text)]
    for i, s in enumerate(starts):
        hdr = text[s:text.index('\n', s + 4)]
        path = text[s:s + 300].split('Compile unit: ', 1)[1].split('\n', 1)[0]
        stem = path.rsplit('.', 1)[0]
        if stem.endswith(BS + want) or stem.endswith(want):
            end = starts[i + 1] if i + 1 < len(starts) else len(text)
            return text[s:end]
    return None


def asm_text(unit):
    p = os.path.join(ROOT, 'build', '2049LE', 'asm', unit + '.s')
    return open(p, encoding='utf-8').read() if os.path.exists(p) else None


def compact_asm(s):
    """Drop the address/bytes comment columns to save space, keep labels and data."""
    out = []
    for ln in s.splitlines():
        m = re.match(r'^/\* ([0-9A-F]{8}) [0-9A-F]{8}  [0-9A-F ]{11} \*/\t(.*)$', ln)
        if m:
            out.append(f"  {m.group(1)[-4:]}  {m.group(2)}")
        elif ln.startswith('# .') or not ln.strip():
            if ln.startswith('# .'):
                out.append(ln)
        else:
            out.append(ln)
    return '\n'.join(out)


if __name__ == '__main__':
    unit = sys.argv[1]
    mode = sys.argv[2] if len(sys.argv) > 2 else ''
    if mode != '--asm-only':
        d = dwarf_section(unit)
        print(d if d else f'(no DWARF section for {unit})')
    if mode != '--dwarf-only':
        a = asm_text(unit)
        print(compact_asm(a) if a else f'(no asm for {unit})')
