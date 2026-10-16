"""Print words from the original DOL: python tools/dolbytes.py <addr hex> <length hex> [f|d]
f/d also prints each word as float / each pair as double, with a C literal that round-trips."""
import struct
import sys

DOL = "orig/2049LE/files/rush_d.dol"


def read(addr, n):
    d = open(DOL, "rb").read()
    offs = struct.unpack(">18I", d[0:0x48])
    addrs = struct.unpack(">18I", d[0x48:0x90])
    sizes = struct.unpack(">18I", d[0x90:0xD8])
    for o, a, s in zip(offs, addrs, sizes):
        if a <= addr and addr + n <= a + s:
            return d[o + addr - a:o + addr - a + n]
    raise SystemExit("address not in an initialised DOL section")


def main():
    addr, n = int(sys.argv[1], 16), int(sys.argv[2], 16)
    mode = sys.argv[3] if len(sys.argv) > 3 else ""
    b = read(addr, n)
    step = 8 if mode == "d" else 4
    for i in range(0, len(b), step):
        w = b[i:i + step]
        line = f"{addr + i:08X}  {w.hex()}"
        if mode == "f" and len(w) == 4:
            line += f"  {struct.unpack('>f', w)[0]!r}f"
        if mode == "d" and len(w) == 8:
            line += f"  {struct.unpack('>d', w)[0]!r}"
        print(line)


if __name__ == "__main__":
    main()
