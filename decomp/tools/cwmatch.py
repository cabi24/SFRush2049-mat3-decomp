"""Compile a C file with many CodeWarrior versions/flag sets and compare functions to rush_d.elf.

Relocated fields are masked before comparing (address halves, small-data offsets,
branch targets). MATCH requires matching relocation offsets, types, named targets
and addends. Missing/unnamed original metadata yields CANDIDATE screening only;
this tool never proves final linked binary equality.

usage: python tools/cwmatch.py <file.c> <func[,func...]> [--versions 2.6,2.7|all]
                               [--flags "<extra flags>" ...] [--show]
"""
import argparse
import itertools
import os
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'tools'))
from elfio import Elf  # noqa: E402

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
COMPILERS = os.path.join(ROOT, 'build', 'compilers', 'GC')
ORIG_ELF = os.path.join(ROOT, '..', 'rush_d.elf')
# The game's confirmed flags (configure.py cflags_game), minus defines; -O4,p etc. are added per run.
BASE = ['-nodefaults', '-proc', 'gekko', '-align', 'powerpc', '-enum', 'int', '-fp', 'hardware',
        '-Cpp_exceptions', 'off', '-pragma', 'cats off', '-pragma', 'warn_notinlined off', '-nosyspath',
        '-RTTI', 'off', '-multibyte', '-use_lmw_stmw', 'on', '-str', 'reuse,readonly', '-lang', 'c',
        '-i', os.path.join(ROOT, 'include'), '-c']
# mask applied to the instruction word for each PPC relocation type
MASKS = {4: 0xffff, 5: 0xffff, 6: 0xffff, 10: 0x3fffffc, 11: 0xfffc, 109: 0x1fffff}

_orig = None


def checked_symbol_bytes(elf, symbol, section, linked=False):
    """Validate the declared section and symbol before any file slicing."""
    start = section['addr'] if linked else 0
    offset = symbol['value'] - start
    size = symbol['size']
    file_start, section_size = section['off'], section['size']
    if (file_start < 0 or section_size < 0 or file_start + section_size > len(elf.d)
            or start < 0 or start + section_size > 0x100000000):
        raise ValueError(f'invalid section file extent: {section["name"]}')
    if (offset < 0 or size < 0 or offset + size > section_size
            or symbol['value'] % 4 or size % 4):
        raise ValueError(f'invalid function extent: {symbol["name"]}')
    return elf.d[file_start + offset:file_start + offset + size]


def orig_funcs():
    global _orig
    if _orig is None:
        e = Elf(ORIG_ELF)
        text = e.by_name['.text']
        funcs = {}
        for s in e.syms:
            if s['type'] == 2 and s['size'] and s['shndx'] == text['idx']:
                raw = checked_symbol_bytes(e, s, text, linked=True)
                funcs.setdefault(s['name'], []).append((s['value'], raw))
        # relocation targets per instruction from the original's leftover .rela.text
        rel = {}
        for off, typ, si, add in e.relocs('.text'):
            if off in rel:
                raise ValueError(f'duplicate original relocation offset {off:#x}')
            width = 2 if typ in (4, 5, 6) else 4
            if not text['addr'] <= off or off + width > text['addr'] + text['size']:
                raise ValueError(f'original relocation outside .text: {off:#x}')
            # ET_EXEC .rela.text offsets are already virtual addresses.
            rel[off] = (typ, e.syms[si]['name'], add)
        _orig = (funcs, rel)
    return _orig


def obj_funcs(path):
    e = Elf(path)
    text = e.by_name['.text']
    out = {}
    rel = {}
    for off, typ, si, add in e.relocs('.text'):
        if off in rel:
            raise ValueError(f'duplicate relocation offset {off:#x}')
        if not 0 <= off < text['size']:
            raise ValueError(f'relocation outside .text: {off:#x}')
        rel[off] = (typ, e.syms[si]['name'], add)
    for s in e.syms:
        if s['type'] == 2 and s['shndx'] == text['idx']:
            if s['name'] in out:
                raise ValueError(f'duplicate function name: {s["name"]}')
            out[s['name']] = (s['value'], checked_symbol_bytes(e, s, text))
    return out, rel


def masked(words, rels, base):
    res = []
    for i, w in enumerate(words):
        r = rels.get(base + i * 4) or rels.get(base + i * 4 + 2)
        res.append(w & ~MASKS.get(r[0], 0) if r else w)
    return res


def relocation_signature(rels, base, size):
    """Keep exact field offsets/types/targets/addends, including halfword sites."""
    return {offset - base: value for offset, value in rels.items()
            if base <= offset < base + size}


def compare_relocations(built, original):
    """Return (definite mismatches, incomplete evidence) for relative signatures."""
    mismatches, incomplete = [], []
    for offset in sorted(set(built) | set(original)):
        br, tr = built.get(offset), original.get(offset)
        if br is None or tr is None:
            incomplete.append(f'+{offset:#x}: relocation metadata missing on one side')
            continue
        if br[0] != tr[0]:
            mismatches.append(f'+{offset:#x}: type {br[0]} vs {tr[0]}')
        if not br[1] or not tr[1]:
            incomplete.append(f'+{offset:#x}: unnamed relocation target')
        elif br[1] != tr[1]:
            # Recognizing a compiler counter is only evidence that identity is
            # unresolved. It never makes differently named targets equivalent.
            static_counter = (re.search(r'\$\d+$', br[1]) and re.search(r'\$\d+$', tr[1])
                              and re.sub(r'\$\d+$', '$', br[1]) == re.sub(r'\$\d+$', '$', tr[1]))
            literal_counter = re.fullmatch(r'@\d+', br[1]) and re.fullmatch(r'@\d+', tr[1])
            if static_counter or literal_counter:
                incomplete.append(f'+{offset:#x}: target identity needs placement/value evidence '
                                  f'({br[1]} vs {tr[1]})')
            else:
                mismatches.append(f'+{offset:#x}: target {br[1]} vs {tr[1]}')
        if br[2] != tr[2]:
            mismatches.append(f'+{offset:#x}: addend {br[2]} vs {tr[2]}')
        if br[0] not in MASKS or tr[0] not in MASKS:
            incomplete.append(f'+{offset:#x}: unsupported relocation type')
    return mismatches, incomplete


def compare(name, built, brel, show=False):
    funcs, orel = orig_funcs()
    if name not in funcs:
        return 'missing-orig'
    if name not in built:
        return 'missing-built'
    bval, bbytes = built[name]
    if len(funcs[name]) != 1:
        return 'ambiguous-orig (multiple functions have this name)'
    oaddr, obytes = funcs[name][0]
    if len(bbytes) != len(obytes):
        return f'size {len(bbytes):#x} vs {len(obytes):#x}'
    if len(bbytes) % 4:
        return 'invalid function extent (not whole PPC instructions)'
    bw = [int.from_bytes(bbytes[i:i + 4], 'big') for i in range(0, len(bbytes), 4)]
    ow = [int.from_bytes(obytes[i:i + 4], 'big') for i in range(0, len(obytes), 4)]
    bsig = relocation_signature(brel, bval, len(bbytes))
    osig = relocation_signature(orel, oaddr, len(obytes))
    for signature in (bsig, osig):
        for offset, (kind, _, _) in signature.items():
            width = 2 if kind in (4, 5, 6) else 4
            alignment = 2 if width == 2 else 0
            if offset % 4 != alignment or offset + width > len(bbytes):
                return 'invalid relocation extent/alignment'
    relocation_errors, incomplete = compare_relocations(bsig, osig)
    if relocation_errors:
        if show:
            for error in relocation_errors[:8]:
                print(f'      relocation {error}')
        return f'{len(relocation_errors)} relocation differences'
    # Apply the union of field masks to BOTH sides. Missing metadata can only
    # yield CANDIDATE, and known type/target/addend differences already failed.
    union = {**osig, **bsig}
    bm = masked(bw, union, 0)
    om = masked(ow, union, 0)
    diff = [i for i, (a, b) in enumerate(zip(bm, om)) if a != b]
    if show and diff:
        for i in diff[:8]:
            print(f"      +{i * 4:#05x}: built {bw[i]:08x} orig {ow[i]:08x}")
    if diff:
        return f'{len(diff)}/{len(bw)} instrs differ'
    if incomplete:
        return 'CANDIDATE (masked equality; relocation identity/metadata needs evidence; full link required)'
    return 'MATCH'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('src')
    ap.add_argument('funcs')
    ap.add_argument('--versions', default='all')
    ap.add_argument('--flags', action='append', default=None)
    ap.add_argument('--show', action='store_true')
    a = ap.parse_args()
    versions = sorted(os.listdir(COMPILERS)) if a.versions == 'all' else a.versions.split(',')
    flagsets = a.flags or ['-O4,p -fp_contract on -inline on']  # the game's confirmed flags
    names = a.funcs.split(',')
    tmp = tempfile.mkdtemp(prefix='cwmatch_')
    for v, fl in itertools.product(versions, flagsets):
        obj = os.path.join(tmp, f"{v}.o")
        cmd = [os.path.join(COMPILERS, v, 'mwcceppc.exe'), *BASE, *fl.split(), a.src, '-o', obj]
        r = subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=60)
        if r.returncode or not os.path.exists(obj):
            err = (r.stdout + r.stderr).strip().splitlines()
            print(f"GC/{v:9} [{fl}] compile failed: {err[-1] if err else r.returncode}")
            continue
        built, brel = obj_funcs(obj)
        res = {n: compare(n, built, brel, a.show) for n in names}
        tag = 'ALL MATCH' if all(x == 'MATCH' for x in res.values()) else ''
        print(f"GC/{v:9} [{fl}] " + '  '.join(f"{n}={x}" for n, x in res.items()) + (f"   <== {tag}" if tag else ''))
        os.remove(obj)


if __name__ == '__main__':
    main()
