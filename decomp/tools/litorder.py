"""Compile a TU with the game's GC/2.6 flags; print its .sdata2 pool and cwmatch results for the given functions.
usage (from decomp/): python tools/litorder.py <src.c> Func1,Func2 [--show]"""
import sys, os, struct, subprocess, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cwmatch
src, funcs = sys.argv[1], sys.argv[2].split(',')
show = '--show' in sys.argv
obj = os.path.join(tempfile.mkdtemp(prefix='lit_'), 'x.o')
cmd = [os.path.join(cwmatch.COMPILERS, '2.6', 'mwcceppc.exe'), *cwmatch.BASE, *'-O4,p -fp_contract on -inline on'.split(), src, '-o', obj]
r = subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=120)
if r.returncode or not os.path.exists(obj):
    print('COMPILE FAILED:', (r.stdout + r.stderr).strip().splitlines()[-3:]); sys.exit(1)
d = open(obj, 'rb').read()
shoff = struct.unpack('>I', d[0x20:0x24])[0]; shentsize, shnum, shstrndx = struct.unpack('>HHH', d[0x2e:0x34])
strtab_off = struct.unpack('>I', d[shoff + shstrndx*shentsize + 0x10: shoff + shstrndx*shentsize + 0x14])[0]
for i in range(shnum):
    s = d[shoff + i*shentsize: shoff + (i+1)*shentsize]
    name, typ, flags, addr, off, size, link, info, align, ent = struct.unpack('>IIIIIIIIII', s)
    n = d[strtab_off+name: d.index(b'\0', strtab_off+name)].decode()
    if n == '.sdata2':
        sd = d[off:off+size]
        print('.sdata2:', ' '.join(sd[j:j+4].hex() for j in range(0, len(sd), 4)))
built, brel = cwmatch.obj_funcs(obj)
for f in funcs:
    print(f, '=', cwmatch.compare(f, built, brel, show))
