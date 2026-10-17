"""Per-unit data-section byte check: the built object's .rodata/.data/.sdata/.sdata2 against the
original ELF bytes at the unit's split ranges, with relocated words masked on both sides.
Catches literal-pool order and data-order differences that objdiff/check.py do not.

usage (from decomp/): python tools/datacheck.py <unit> [...]    e.g. GameSource/src/game
Reads build/2049LE/src/<unit>.o (run ninja or check.py first) and config/2049LE/splits.txt.
"""
import os, re, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))  # elfio.py
from elfio import Elf  # noqa: E402

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')  # repo root (rush_d.elf lives there, git-ignored)
ORIG = Elf(os.path.join(ROOT, 'rush_d.elf'))
SECS = ['.rodata', '.data', '.sdata', '.sdata2']
MASK = {1: 0xffffffff, 4: 0xffff, 5: 0xffff, 6: 0xffff, 10: 0x3fffffc, 11: 0xfffc, 109: 0x1fffff}


def splits():
    out, cur = {}, None
    for line in open(os.path.join(ROOT, 'decomp', 'config', '2049LE', 'splits.txt')):
        m = re.match(r'^([A-Za-z].*\.c(?:pp)?):\s*$', line)
        if m:
            cur = m.group(1)[:-2] if m.group(1).endswith('.c') else m.group(1)[:-4]
            out[cur] = {}
            continue
        m = re.match(r'\s+(\.\w+)\s+start:(0x[0-9A-Fa-f]+)\s+end:(0x[0-9A-Fa-f]+)', line)
        if m and cur:
            out[cur][m.group(1)] = (int(m.group(2), 16), int(m.group(3), 16))
    return out


def orig_words_masked(sec, lo, hi):
    s = ORIG.by_name[sec]
    raw = ORIG.d[s['off'] + (lo - s['addr']): s['off'] + (hi - s['addr'])]
    rel = {}
    for off, typ, si, add in ORIG.relocs(sec):
        if lo <= off < hi:
            rel[off - lo] = typ
    return raw, rel


def built_words_masked(obj, sec):
    e = Elf(obj)
    if sec not in e.by_name:
        return b'', {}
    s = e.by_name[sec]
    raw = bytes(e.data(sec)) if s['type'] != 8 else b'\0' * s['size']
    rel = {off: typ for off, typ, si, add in e.relocs(sec)}
    # MSL's inline sqrtf leaves weak `_half$...`/`_three$...` objects in .sdata2; the linker merges
    # them into one copy elsewhere, so drop their bytes (and shift later relocation offsets).
    cut = sorted((y['value'], y['size']) for y in e.syms if y['shndx'] == s['idx'] and
                 (y['name'].startswith('_half$') or y['name'].startswith('_three$')))
    for off, size in reversed(cut):
        raw = raw[:off] + raw[off + size:]
        rel = {(k - size if k >= off + size else k): v for k, v in rel.items() if not (off <= k < off + size)}
    return raw, rel


def mask(raw, rel):
    w = bytearray(raw)
    for off, typ in rel.items():
        m = MASK.get(typ, 0xffffffff)
        if m == 0xffff:
            if off + 2 <= len(w):
                w[off] = 0; w[off + 1] = 0
        else:
            if off + 4 <= len(w):
                v = int.from_bytes(w[off:off + 4], 'big') & ~m & 0xffffffff
                w[off:off + 4] = v.to_bytes(4, 'big')
    return bytes(w)


def check(unit):
    sp = splits().get(unit)
    obj = os.path.join(ROOT, 'decomp', 'build', '2049LE', 'src', unit + '.o')
    if sp is None or not os.path.exists(obj):
        print(f'== {unit}: missing split or object ({obj})'); return False
    ok_all = True
    for sec in SECS:
        if sec not in sp:
            continue
        lo, hi = sp[sec]
        oraw, orel = orig_words_masked(sec, lo, hi)
        braw, brel = built_words_masked(obj, sec)
        # the split may include trailing alignment padding beyond the object's size
        n = min(len(oraw), len(braw))
        om = mask(oraw[:n], {k: v for k, v in orel.items() if k < n})
        bm = mask(braw[:n], {k: v for k, v in brel.items() if k < n})
        # also mask built relocation sites in the original and vice versa (union)
        om = mask(om, {k: v for k, v in brel.items() if k < n})
        bm = mask(bm, {k: v for k, v in orel.items() if k < n})
        diffs = [i for i in range(0, n, 4) if om[i:i + 4] != bm[i:i + 4]]
        tail = oraw[n:] if len(oraw) > len(braw) else b''
        size_note = '' if len(oraw) == len(braw) else f' (size target {len(oraw):#x} vs built {len(braw):#x}; split tail {tail.hex()[:16]})'
        if diffs or (len(braw) > len(oraw)):
            ok_all = False
            first = diffs[0] if diffs else n
            print(f'   {sec:8s} DIFF {len(diffs)} words; first at +{first:#x}: target {oraw[first:first+8].hex()} built {braw[first:first+8].hex()}{size_note}')
        else:
            print(f'   {sec:8s} ok ({n:#x} bytes compared){size_note}')
    print(f'== {unit}: {"DATA OK" if ok_all else "DATA DIFFERS"}')
    return ok_all


if __name__ == '__main__':
    for u in sys.argv[1:]:
        check(u)
