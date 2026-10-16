import sys, os, struct, hashlib
sys.path.insert(0, os.path.dirname(__file__))
from elfio import Elf

iso_path, elf_path = sys.argv[1], sys.argv[2]
e = Elf(elf_path)
print("ELF entry:", hex(struct.unpack_from('>I', e.d, 24)[0]))
print(f"{'section':12} {'addr':>10} {'size':>10} {'file_off':>10}")
for s in e.secs:
    if s['name'] and s['addr']:
        print(f"{s['name']:12} {s['addr']:#010x} {s['size']:#10x} {s['off']:#10x}")

# Locate rush_d.dol through the disc's file table
iso = open(iso_path, 'rb')
h = iso.read(0x440)
dol_main, fst_off, fst_sz = struct.unpack_from('>III', h, 0x420)
iso.seek(fst_off)
fst = iso.read(fst_sz)
count = struct.unpack_from('>I', fst, 8)[0]
strtab = count * 12
loc = {}
for i in range(1, count):
    fn, a, b = struct.unpack_from('>III', fst, i * 12)
    if fn >> 24 == 0:
        o = strtab + (fn & 0xffffff)
        loc[fst[o:fst.index(b'\0', o)].decode('latin1')] = (a, b)


def dol_segments(off):
    iso.seek(off)
    hd = iso.read(0x100)
    offs = struct.unpack_from('>18I', hd, 0)
    addrs = struct.unpack_from('>18I', hd, 0x48)
    sizes = struct.unpack_from('>18I', hd, 0x90)
    entry = struct.unpack_from('>I', hd, 0xe0)[0]
    segs = []
    for i in range(18):
        if sizes[i]:
            iso.seek(off + offs[i])
            segs.append((('T' if i < 7 else 'D') + str(i if i < 7 else i - 7), addrs[i], iso.read(sizes[i])))
    return entry, segs


for label, off in (('rush_d.dol', loc['rush_d.dol'][0]), ('boot main.dol', dol_main)):
    entry, segs = dol_segments(off)
    print(f"\n{label}: entry {entry:#x}")
    for name, addr, data in segs:
        # compare against ELF bytes at the same virtual address
        match = None
        for s in e.secs:
            if s['addr'] and s['type'] != 8 and s['addr'] <= addr < s['addr'] + s['size']:
                eb = e.d[s['off'] + addr - s['addr']: s['off'] + addr - s['addr'] + len(data)]
                match = (s['name'], eb == data)
        print(f"   {name:4} {addr:#010x} size {len(data):#8x}  ELF section/identical: {match}")
