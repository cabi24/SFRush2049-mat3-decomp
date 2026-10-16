import struct, sys, collections

f = open(sys.argv[1], 'rb')
h = f.read(0x440)
dol_off, fst_off, fst_sz = struct.unpack_from('>III', h, 0x420)
f.seek(fst_off)
fst = f.read(fst_sz)
count = struct.unpack_from('>I', fst, 8)[0]
strtab = count * 12

def name_at(o):
    e = fst.index(b'\0', strtab + o)
    return fst[strtab + o:e].decode('latin1')

files = []
dirs = {0: ''}
stack = [(count, '')]  # (end index, path)
for i in range(1, count):
    while stack and i >= stack[-1][0]:
        stack.pop()
    flags_name, a, b = struct.unpack_from('>III', fst, i * 12)
    is_dir = flags_name >> 24
    nm = name_at(flags_name & 0xffffff)
    parent = stack[-1][1] if stack else ''
    path = f"{parent}/{nm}" if parent else nm
    if is_dir:
        stack.append((b, path))
    else:
        files.append((path, a, b))

def magic(off):
    f.seek(off)
    return f.read(4)

print(f"{len(files)} files on disc (+ main.dol @ {dol_off:#x})")
f.seek(dol_off)
dh = f.read(0x100)
text = struct.unpack_from('>7I', dh, 0x90)
data = struct.unpack_from('>11I', dh, 0xac)
dol_size = max(o + s for o, s in zip(struct.unpack_from('>18I', dh, 0), struct.unpack_from('>18I', dh, 0x90)) if o)
print(f"main.dol size {dol_size:,} bytes, entry {struct.unpack_from('>I', dh, 0xe0)[0]:#x}")

exts = collections.Counter()
for p, o, s in files:
    exts[p.rsplit('.', 1)[-1].lower() if '.' in p.rsplit('/', 1)[-1] else '(none)'] += 1
print("by extension:", dict(exts.most_common()))

elfs = []
for p, o, s in files:
    if s >= 4 and magic(o) == b'\x7fELF':
        elfs.append((p, o, s))
print(f"\nfiles that ARE ELF (magic at start): {len(elfs)}")
for e in elfs:
    print("  ", e[0], f"{e[2]:,} bytes @ {e[1]:#x}")

interesting = [x for x in files if x[0].lower().rsplit('.', 1)[-1] in ('elf', 'dol', 'rel', 'map', 'sym', 'o', 'a', 'so', 'plf', 'exe', 'dll', 'pdb', 'bin', 'rso', 'sel')]
print("\nexecutable-ish names:")
for p, o, s in interesting:
    print(f"   {p}  {s:,} bytes  magic={magic(o).hex()}")
print("\ntop-level layout:")
top = collections.defaultdict(lambda: [0, 0])
for p, o, s in files:
    k = p.split('/')[0] if '/' in p else '(root) ' + p
    top[k][0] += 1
    top[k][1] += s
for k, (c, s) in sorted(top.items(), key=lambda kv: -kv[1][1]):
    print(f"   {k:45} {c:5} files {s / 2**20:8.1f} MiB")
