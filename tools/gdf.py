import re, mmap, struct, sys, zlib, collections, csv

BS = chr(92)
p = sys.argv[1]
f = open(p, 'rb')
m = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)

# Trailer near EOF holds the record-table size (multiple of 200) and block-table size (multiple of 8)
# Last record ends right before the trailer; find it via the last timestamp
tail = m[len(m) - 4096:]
d = [x.start() for x in re.finditer(rb'(?:19|20)[0-9]{2}-[0-9]{2}-[0-9]{2} [0-9:]{8}', tail)][-1] + len(m) - 4096
end = d - 0xa8 + 200

tbl_sz = blk_sz = None
for o in range(end, len(m) - 11):
    v = struct.unpack_from('<I', m, o)[0]
    w = struct.unpack_from('<I', m, o + 8)[0]
    s = end - v
    if v and v % 200 == 0 and v < end and w and w % 8 == 0 and w < s and m[s + 4 + 0xa4:s + 4 + 0xa6] in (b'19', b'20'):
        tbl_sz, blk_sz = v, w
        break
start = end - tbl_sz
n = tbl_sz // 200
bstart = start - blk_sz
print(f"records={n} table@{start:#x} blocks={blk_sz // 8} blocktable@{bstart:#x} trailer@{end:#x}")
blocks = [struct.unpack_from('<II', m, bstart + i * 8) for i in range(blk_sz // 8)]
print("first block entries:", [(hex(a), b) for a, b in blocks[:3]], "last:", [(hex(a), b) for a, b in blocks[-2:]])

recs = []
for i in range(n):
    r = start + i * 200
    name = m[r + 4:r + 132].split(b'\0')[0].decode('latin1')
    h, size = struct.unpack_from('<II', m, r + 132)
    ext = m[r + 140:r + 152].split(b'\0')[0].decode('latin1')
    b0, off, b1 = struct.unpack_from('<III', m, r + 4 + 0x98)
    date = m[r + 4 + 0xa4:r + 4 + 0xb8].split(b'\0')[0].decode('latin1')
    recs.append(dict(name=name, ext=ext.lower(), size=size, date=date,
                     block_start=b0, block_offset=off, block_end=b1, hash=f"{h:08x}"))

with open(sys.argv[2], 'w', newline='') as o:
    w = csv.DictWriter(o, fieldnames=list(recs[0]))
    w.writeheader()
    w.writerows(recs)

tot = sum(r['size'] for r in recs)
print(f"total uncompressed={tot:,} bytes ({tot / 2**20:.1f} MiB) vs archive {len(m) / 2**20:.1f} MiB")

byext = collections.defaultdict(lambda: [0, 0])
for r in recs:
    byext[r['ext']][0] += 1
    byext[r['ext']][1] += r['size']
print("\nBy type:")
for e, (c, s) in sorted(byext.items(), key=lambda kv: -kv[1][1]):
    print(f"  {e or '(none)':6} {c:5} files {s / 2**20:9.1f} MiB")

top = collections.defaultdict(lambda: [0, 0])
for r in recs:
    parts = r['name'].split(BS)
    k = BS.join(parts[:2]) if len(parts) > 2 else parts[0]
    top[k][0] += 1
    top[k][1] += r['size']
print("\nBy folder:")
for k, (c, s) in sorted(top.items(), key=lambda kv: -kv[1][1])[:45]:
    print(f"  {k:40} {c:5} files {s / 2**20:8.1f} MiB")

dates = sorted(r['date'] for r in recs if r['date'])
print("\nDate range:", dates[0], "->", dates[-1])
print("By year:", dict(sorted(collections.Counter(x[:4] for x in dates).items())))

odd = [r for r in recs if r['ext'] not in ('dds', 'lzs', 'dat', 'bin', 'scc', 'wav', 'xtex', 'xsb', 'xsh', 'xss')]
print("\nOther files:")
for r in odd:
    print("  ", r['name'], r['size'], r['date'])
print("\n.bin files sample:", [r['name'] for r in recs if r['ext'] == 'bin'][:12])
print(".dat files sample:", [r['name'] for r in recs if r['ext'] == 'dat'][:12])

print("\nblock0 head:", m[0:16].hex())
for wb in (15, -15, 31):
    try:
        out = zlib.decompressobj(wb).decompress(m[0:65536])
        print(f"zlib wbits={wb}: OK ->", out[:32])
        break
    except Exception as e:
        print(f"zlib wbits={wb}:", e)
