"""Compare two DOL files segment by segment and summarise where their bytes differ.

usage: python tools/doldiff.py <built.dol> <original.dol> [symbols.txt]
With symbols.txt, each differing range is labelled with the enclosing symbol.
"""
import bisect
import re
import struct
import sys


def segments(path):
    d = open(path, 'rb').read()
    offs = struct.unpack_from('>18I', d, 0)
    addrs = struct.unpack_from('>18I', d, 0x48)
    sizes = struct.unpack_from('>18I', d, 0x90)
    segs = {}
    for i in range(18):
        if sizes[i]:
            segs[('T%d' % i) if i < 7 else ('D%d' % (i - 7))] = (addrs[i], d[offs[i]:offs[i] + sizes[i]])
    bss = struct.unpack_from('>II', d, 0xd8)
    return segs, bss, struct.unpack_from('>I', d, 0xe0)[0]


def main():
    a_segs, a_bss, a_ent = segments(sys.argv[1])
    b_segs, b_bss, b_ent = segments(sys.argv[2])
    syms = []
    if len(sys.argv) > 3:
        for ln in open(sys.argv[3], encoding='utf-8'):
            m = re.match(r'^(\S+) = (\S+):(0x[0-9A-Fa-f]+);', ln)
            if m:
                syms.append((int(m.group(3), 16), m.group(1)))
        syms.sort()
    saddr = [s[0] for s in syms]

    def label(addr):
        i = bisect.bisect_right(saddr, addr) - 1
        return f"{syms[i][1]}+{addr - syms[i][0]:#x}" if i >= 0 else '?'

    print(f"entry {a_ent:#x} vs {b_ent:#x}; bss {a_bss[0]:#x}+{a_bss[1]:#x} vs {b_bss[0]:#x}+{b_bss[1]:#x}")
    for k in sorted(set(a_segs) | set(b_segs)):
        a = a_segs.get(k)
        b = b_segs.get(k)
        if not a or not b:
            print(f"{k}: only in {'built' if a else 'original'}")
            continue
        (aa, ad), (ba, bd) = a, b
        n = min(len(ad), len(bd))
        diffs = [i for i in range(n) if ad[i] != bd[i]]
        print(f"{k}: built {aa:#010x}+{len(ad):#x}  orig {ba:#010x}+{len(bd):#x}  differing bytes {len(diffs)}")
        runs = []
        for i in diffs:
            if runs and i - runs[-1][1] <= 16:
                runs[-1][1] = i
            else:
                runs.append([i, i])
        for s, e in runs[:12]:
            addr = ba + s
            print(f"    {addr:#010x}..{ba + e:#010x} ({e - s + 1} B) {label(addr) if syms else ''}"
                  f"  built={ad[s:s + 8].hex()} orig={bd[s:s + 8].hex()}")
        if len(runs) > 12:
            print(f"    ... {len(runs) - 12} more ranges")


if __name__ == '__main__':
    main()
