import struct, sys, csv, hashlib, collections

iso_path, off, size, out_elf, out_csv = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3]), sys.argv[4], sys.argv[5]

with open(iso_path, 'rb') as iso:
    iso.seek(off)
    d = iso.read(size)
assert d[:4] == b'\x7fELF' and len(d) == size
with open(out_elf, 'wb') as o:
    o.write(d)
print(f"wrote {out_elf}: {len(d):,} bytes, sha1 {hashlib.sha1(d).hexdigest()}")

en = '>' if d[5] == 2 else '<'
shoff = struct.unpack_from(en + 'I', d, 0x20)[0]
shentsz, shnum, shstrndx = struct.unpack_from(en + 'HHH', d, 0x2e)
secs = [struct.unpack_from(en + 'IIIIIIIIII', d, shoff + i * shentsz) for i in range(shnum)]
sso = secs[shstrndx][4]

def cstr(base, o):
    return d[base + o:d.index(b'\0', base + o)].decode('latin1')

secnames = [cstr(sso, s[0]) for s in secs]
print("sections:", ', '.join(f"{n}({s[5]:,})" for n, s in zip(secnames, secs) if n))

TYPES = {0: 'NOTYPE', 1: 'OBJECT', 2: 'FUNC', 3: 'SECTION', 4: 'FILE'}
BINDS = {0: 'LOCAL', 1: 'GLOBAL', 2: 'WEAK'}
VIS = {0: 'DEFAULT', 1: 'INTERNAL', 2: 'HIDDEN', 3: 'PROTECTED'}
SPECIAL = {0: 'UND', 0xfff1: 'ABS', 0xfff2: 'COMMON'}

rows = []
cur_file = ""
for si, s in enumerate(secs):
    if s[1] != 2:  # SHT_SYMTAB
        continue
    strbase = secs[s[6]][4]
    ent = s[9] or 16
    for k in range(s[5] // ent):
        st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from(en + 'IIIBBH', d, s[4] + k * ent)
        if k == 0:
            continue
        sec = SPECIAL.get(st_shndx, secnames[st_shndx] if st_shndx < len(secnames) else str(st_shndx))
        name = cstr(strbase, st_name)
        typ, bind = st_info & 0xf, st_info >> 4
        if typ == 4:
            cur_file = name
        # Local symbols follow their FILE symbol; globals come after all locals and carry no file
        rows.append(dict(index=k, name=name, address=f"0x{st_value:08X}", size=st_size,
                         type=TYPES.get(typ, typ), bind=BINDS.get(bind, bind),
                         visibility=VIS.get(st_other & 3), section=sec,
                         source_file=cur_file if bind == 0 and typ != 4 else ''))

with open(out_csv, 'w', newline='', encoding='utf-8') as o:
    w = csv.DictWriter(o, fieldnames=list(rows[0]))
    w.writeheader()
    w.writerows(rows)
print(f"wrote {out_csv}: {len(rows):,} symbols")
print("by type:", dict(collections.Counter(r['type'] for r in rows).most_common()))
print("by bind:", dict(collections.Counter(r['bind'] for r in rows).most_common()))
print("by section:", dict(collections.Counter(r['section'] for r in rows).most_common(10)))
files = [r['name'] for r in rows if r['type'] == 'FILE']
print(f"FILE symbols: {len(files)}", files[:8])
named = [r for r in rows if r['name']]
print(f"unnamed symbols: {len(rows) - len(named)}")
mangled = sum(1 for r in rows if '__' in r['name'][1:] and r['type'] == 'FUNC')
print(f"C++-mangled function names (approx): {mangled}")
