"""Minimal big-endian ELF32 reader shared by the rush_d.elf scripts."""
import struct


class Elf:
    def __init__(self, path):
        self.d = open(path, 'rb').read()
        d = self.d
        assert d[:4] == b'\x7fELF' and d[4] == 1 and d[5] == 2, "expects ELF32 big-endian"
        shoff = struct.unpack_from('>I', d, 0x20)[0]
        shentsz, shnum, shstrndx = struct.unpack_from('>HHH', d, 0x2e)
        self.secs = []
        raw = [struct.unpack_from('>IIIIIIIIII', d, shoff + i * shentsz) for i in range(shnum)]
        sso = raw[shstrndx][4]
        for i, s in enumerate(raw):
            name = d[sso + s[0]:d.index(b'\0', sso + s[0])].decode('latin1')
            self.secs.append(dict(idx=i, name=name, type=s[1], flags=s[2], addr=s[3], off=s[4],
                                  size=s[5], link=s[6], info=s[7], align=s[8], entsize=s[9]))
        self.by_name = {s['name']: s for s in self.secs}
        self.syms = self._symbols()

    def data(self, name):
        s = self.by_name[name]
        return bytearray(self.d[s['off']:s['off'] + s['size']])

    def _symbols(self):
        st = next(s for s in self.secs if s['type'] == 2)
        strbase = self.secs[st['link']]['off']
        out = []
        for k in range(st['size'] // 16):
            nm, val, size, info, other, shndx = struct.unpack_from('>IIIBBH', self.d, st['off'] + k * 16)
            name = self.d[strbase + nm:self.d.index(b'\0', strbase + nm)].decode('latin1')
            out.append(dict(index=k, name=name, value=val, size=size, type=info & 0xf, bind=info >> 4,
                            other=other, shndx=shndx))
        return out

    def relocs(self, target):
        """Yield (offset, type, sym_index, addend) for the .rela section that applies to `target`."""
        s = self.by_name.get('.rela' + target)
        if not s:
            return
        for k in range(s['size'] // 12):
            off, info, add = struct.unpack_from('>IIi', self.d, s['off'] + k * 12)
            yield off, info & 0xff, info >> 8, add

    def relocated(self, target):
        """Section bytes with R_PPC_ADDR32 relocations applied (S + A)."""
        buf = self.data(target)
        kinds = {}
        for off, typ, si, add in self.relocs(target):
            kinds[typ] = kinds.get(typ, 0) + 1
            if typ == 1:  # R_PPC_ADDR32
                struct.pack_into('>I', buf, off, (self.syms[si]['value'] + add) & 0xffffffff)
        return buf, kinds
