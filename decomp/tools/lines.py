"""Print a target function's instructions grouped by original source line (DWARF v1 .line table).

usage (from decomp/): python tools/lines.py <function> [...]      e.g. GetCarsIRColor Blits
The line table gives statement boundaries and evaluation order straight from the original source:
each source line that emitted code appears with its instructions from build/2049LE/asm/<unit>.s.
"""
import csv, glob, os, re, struct, sys
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from elfio import Elf  # noqa: E402


def line_table():
    e = Elf(os.path.join(ROOT, 'rush_d.elf'))
    d = bytes(e.data('.line'))
    out = {}  # address -> line
    off = 0
    while off + 8 <= len(d):
        ln, base = struct.unpack('>II', d[off:off + 8])
        for i in range((ln - 8) // 10):
            l, c, delta = struct.unpack('>IHI', d[off + 8 + i * 10:off + 18 + i * 10])
            out[base + delta] = l
        off += ln
    return out


def main():
    funcs = {r['name']: r for r in csv.DictReader(open(os.path.join(ROOT, 'rush_d_functions.csv'), encoding='utf-8', errors='replace'))
             if r['address'] != '0x00000000'}
    lines = line_table()
    for name in sys.argv[1:]:
        f = funcs[name]
        lo, hi = int(f['address'], 16), int(f['end'], 16) if 'end' in f else int(f['address'], 16) + int(f['size'])
        unit = f['source_file'][:-2]
        asm = None
        for p in glob.glob(os.path.join(ROOT, 'decomp', 'build', '2049LE', 'asm', '**', unit + '.s'), recursive=True):
            asm = p
        ins = {}
        if asm:
            for m in re.finditer(r'^/\* ([0-9A-F]{8}) [0-9A-F]{8}  [0-9A-F ]{11} \*/\t(.*)$', open(asm, encoding='utf-8', errors='replace').read(), re.M):
                a = int(m.group(1), 16)
                if lo <= a < hi:
                    ins[a] = m.group(2).strip()
        print(f'== {name} {f["source_file"]} lines {f["line"]}-{f["end_line"]} {lo:#x}-{hi:#x} '
              f'({(hi - lo) // 4} instructions, {sum(1 for a in lines if lo <= a < hi)} line entries)')
        cur = None
        for a in range(lo, hi, 4):
            if a in lines and lines[a] != cur:
                cur = lines[a]
                print(f'--- line {cur}')
            print(f'  {a:#x}  {ins.get(a, "?")}')


if __name__ == '__main__':
    main()
