"""Compile a TU with the game's GC/2.6 flags and print its .sbss/.bss/.data symbol order (uninitialised
statics emit in reverse declaration order, initialised ones in order at their declaration point).
usage (from decomp/): python tools/sbssorder.py <src.c>"""
import sys, os, subprocess, tempfile
_d = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, _d); sys.path.insert(0, os.path.join(_d, '..', '..', 'tools'))  # cwmatch.py, elfio.py
import cwmatch
from elfio import Elf
src = sys.argv[1]
obj = os.path.join(tempfile.mkdtemp(prefix='sb_'), 'x.o')
cmd = [os.path.join(cwmatch.COMPILERS, '2.6', 'mwcceppc.exe'), *cwmatch.BASE, *'-O4,p -fp_contract on -inline on'.split(), src, '-o', obj]
r = subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=180)
if r.returncode: print('COMPILE FAILED', (r.stdout+r.stderr).strip().splitlines()[-3:]); sys.exit(1)
e = Elf(obj)
for secname in ['.sbss', '.bss', '.data']:
    if secname not in e.by_name: continue
    s = e.by_name[secname]
    print(secname, ' '.join(f"{y['name'].split('$')[0]}@{y['value']}" for y in sorted((y for y in e.syms if y['shndx'] == s['idx'] and y['name'] and y['type'] == 1), key=lambda y: y['value'])))
