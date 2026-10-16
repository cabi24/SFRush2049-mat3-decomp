"""Compare whole object files (all sections + symbol order) produced by several CW versions.

usage: python tools/cwobjdiff.py "<flags>" <versions,csv> <file.c> [file.c ...]
Prints, per source file, which versions emit byte-different sections or symbol tables.
"""
import collections
import hashlib
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
import cwmatch  # noqa: E402
from elfio import Elf  # noqa: E402

flags, versions, srcs = sys.argv[1].split(), sys.argv[2].split(','), sys.argv[3:]
tmp = tempfile.mkdtemp(prefix='cwobj_')
for src in srcs:
    sig = collections.defaultdict(dict)
    for v in versions:
        obj = os.path.join(tmp, f"{v}.o")
        cmd = [os.path.join(cwmatch.COMPILERS, v, 'mwcceppc.exe'), *cwmatch.BASE, *flags, src, '-o', obj]
        r = subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=60)
        if r.returncode:
            sig['(compile)'][v] = 'FAIL'
            continue
        e = Elf(obj)
        for s in e.secs:
            if s['name'] in ('', '.comment', '.shstrtab', '.strtab', '.symtab'):
                continue
            data = e.d[s['off']:s['off'] + s['size']] if s['type'] != 8 else str(s['size']).encode()
            sig[s['name']][v] = hashlib.sha1(data).hexdigest()[:8]
        order = ','.join(f"{x['name']}:{x['shndx']}:{x['value']}" for x in e.syms)
        sig['(symbols)'][v] = hashlib.sha1(order.encode()).hexdigest()[:8]
    diffs = {k: d for k, d in sig.items() if len(set(d.values())) > 1}
    name = os.path.basename(src)
    if not diffs:
        print(f"{name:14} identical across {','.join(versions)}")
    for k, d in diffs.items():
        groups = collections.defaultdict(list)
        for v, h in d.items():
            groups[h].append(v)
        print(f"{name:14} {k:12} " + ' | '.join(','.join(g) for g in groups.values()))
