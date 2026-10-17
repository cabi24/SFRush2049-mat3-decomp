"""Decode CodeWarrior DWARF v1 type information from rush_d.elf into C headers.

usage: python tools/dwarftypes.py rush_d.elf <out_dir> [splits.txt]

Writes into <out_dir>:
  rush_types.h   every struct/union/enum, deduplicated by layout, in dependency order
  rush_protos.h  prototypes for all global functions + extern declarations for all globals
  rush_decls.json per compile unit: its functions (signature, statics, locals) and variables,
                 as C text using the names in rush_types.h (used by the skeleton generator)

Anonymous types get synthetic names (anon_<firstmember>_<secondmember>_<size>); tagged types
keep their tag (tag_2, tag_3 ... when one tag has several layouts across files).
"""
import collections
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from elfio import Elf  # noqa: E402

TAG = {0x01: 'array', 0x04: 'enum', 0x05: 'param', 0x06: 'gfunc', 0x07: 'gvar', 0x0b: 'block', 0x0c: 'lvar',
       0x0d: 'member', 0x11: 'cu', 0x13: 'struct', 0x14: 'sfunc', 0x15: 'functype', 0x17: 'union',
       0x18: 'varargs', 0x0f: 'pointer_type', 0x16: 'typedef', 0x02: 'class', 0x1d: 'inlined'}
FT = {0x1: 'char', 0x2: 'signed char', 0x3: 'unsigned char', 0x4: 'short', 0x5: 'signed short',
      0x6: 'unsigned short', 0x7: 'int', 0x8: 'signed int', 0x9: 'unsigned int', 0xa: 'long',
      0xb: 'signed long', 0xc: 'unsigned long', 0xd: 'void *', 0xe: 'float', 0xf: 'double',
      0x10: 'long double', 0x14: 'void', 0x15: 'bool', 0x8008: 'long long', 0x8108: 'signed long long',
      0x8208: 'unsigned long long'}
FT_SIZE = {'char': 1, 'signed char': 1, 'unsigned char': 1, 'bool': 1, 'short': 2, 'signed short': 2,
           'unsigned short': 2, 'int': 4, 'signed int': 4, 'unsigned int': 4, 'long': 4, 'signed long': 4,
           'unsigned long': 4, 'void *': 4, 'float': 4, 'double': 8, 'long double': 8, 'void': 0,
           'long long': 8, 'signed long long': 8, 'unsigned long long': 8}
AT_SIBLING, AT_LOCATION, AT_NAME = 0x0012, 0x0023, 0x0038
AT_FUND, AT_MODFUND, AT_UDT, AT_MODUDT = 0x0055, 0x0063, 0x0072, 0x0083
AT_SUBSCR, AT_BYTESIZE, AT_BITOFF, AT_BITSIZE, AT_ELEMLIST = 0x00a3, 0x00b6, 0x00c5, 0x00d6, 0x00f4
AT_LOWPC, AT_HIGHPC = 0x0111, 0x0121


def parse_attrs(buf, o, end):
    attrs = {}
    while o + 2 <= end:
        at = struct.unpack_from('>H', buf, o)[0]
        if at == 0:
            break
        o += 2
        form = at & 0xf
        if form in (1, 2, 6):
            v = struct.unpack_from('>I', buf, o)[0]; o += 4
        elif form == 5:
            v = struct.unpack_from('>H', buf, o)[0]; o += 2
        elif form == 7:
            v = struct.unpack_from('>Q', buf, o)[0]; o += 8
        elif form == 3:
            n = struct.unpack_from('>H', buf, o)[0]; v = bytes(buf[o + 2:o + 2 + n]); o += 2 + n
        elif form == 4:
            n = struct.unpack_from('>I', buf, o)[0]; v = bytes(buf[o + 4:o + 4 + n]); o += 4 + n
        elif form == 8:
            e = buf.index(0, o); v = bytes(buf[o:e]).decode('latin1'); o = e + 1
        else:
            raise ValueError(f"form {form} attr {at:#x} at {o:#x}")
        attrs[at] = v
    return attrs


class Die:
    __slots__ = ('off', 'tag', 'a', 'children', 'end', 'cu')

    def __init__(self, off, tag, a, end):
        self.off, self.tag, self.a, self.end, self.children, self.cu = off, tag, a, end, [], None


def read_dies(dbg):
    """All DIEs by offset, with children attached using sibling links."""
    dies, order = {}, []
    o = 0
    while o < len(dbg):
        ln = struct.unpack_from('>I', dbg, o)[0]
        if ln < 8:
            o += max(ln, 4)
            continue
        tag = struct.unpack_from('>H', dbg, o + 4)[0]
        d = Die(o, TAG.get(tag, f'tag{tag:#x}'), parse_attrs(dbg, o + 6, o + ln), o + ln)
        dies[o] = d
        order.append(d)
        o += ln
    # nesting: a DIE's children start right after it and run until its sibling offset
    stack = []
    cu = None
    for d in order:
        while stack and d.off >= stack[-1][1]:
            stack.pop()
        if d.tag == 'cu':
            cu = d
            stack = []
        d.cu = cu
        if stack:
            stack[-1][0].children.append(d)
        sib = d.a.get(AT_SIBLING)
        if sib and sib > d.end and d.tag != 'padding':
            stack.append((d, sib))
    return dies, order


# ---------------------------------------------------------------- type model
class T:
    """kind: base|ptr|const|volatile|array|func|struct|union|enum|unknown"""
    __slots__ = ('kind', 'name', 'sub', 'n', 'params', 'varargs', 'die')

    def __init__(self, kind, name=None, sub=None, n=None, params=None, varargs=False, die=None):
        self.kind, self.name, self.sub, self.n, self.params, self.varargs, self.die = kind, name, sub, n, params, varargs, die


class Decoder:
    def __init__(self, dies):
        self.dies = dies
        self.cache = {}

    def type_of(self, a):
        if AT_FUND in a:
            return T('base', FT.get(a[AT_FUND], f'ft_{a[AT_FUND]:x}'))
        if AT_UDT in a:
            return self.udt(a[AT_UDT])
        if AT_MODFUND in a:
            b = a[AT_MODFUND]
            mods, base = b[:-2], T('base', FT.get(struct.unpack('>H', b[-2:])[0], 'int'))
            return self.apply(mods, base)
        if AT_MODUDT in a:
            b = a[AT_MODUDT]
            mods, base = b[:-4], self.udt(struct.unpack('>I', b[-4:])[0])
            return self.apply(mods, base)
        return T('base', 'void')

    @staticmethod
    def apply(mods, base):
        t = base
        for m in reversed(mods):  # first modifier is outermost
            t = T({1: 'ptr', 2: 'ptr', 3: 'const', 4: 'volatile'}.get(m, 'ptr'), sub=t)
        return t

    def udt(self, off):
        if off in self.cache:
            return self.cache[off]
        d = self.dies.get(off)
        if d is None:
            return T('base', 'int')
        if d.tag in ('struct', 'union', 'class'):
            t = T('struct' if d.tag != 'union' else 'union', d.a.get(AT_NAME), die=d)
            self.cache[off] = t
            return t
        if d.tag == 'enum':
            t = T('enum', d.a.get(AT_NAME), die=d)
            self.cache[off] = t
            return t
        if d.tag == 'array':
            t = self.array(d)
            self.cache[off] = t
            return t
        if d.tag == 'functype':
            t = T('func', sub=self.type_of(d.a), params=[self.type_of(c.a) for c in d.children if c.tag == 'param'],
                  varargs=any(c.tag == 'varargs' for c in d.children))
            self.cache[off] = t
            return t
        return T('base', 'int')

    def array(self, d):
        b = d.a.get(AT_SUBSCR, b'')
        dims, i, elem = [], 0, T('base', 'char')
        while i < len(b):
            fmt = b[i]; i += 1
            if fmt == 0:  # FT_C_C: fund type, low, high
                lo, hi = struct.unpack_from('>ii', b, i + 2); i += 10
                dims.append(hi - lo + 1)
            elif fmt == 4:  # UT_C_C
                lo, hi = struct.unpack_from('>ii', b, i + 4); i += 12
                dims.append(hi - lo + 1)
            elif fmt == 8:  # ET: element type attribute follows
                at = struct.unpack_from('>H', b, i)[0]
                sub = parse_attrs(b, i, len(b))
                elem = self.type_of(sub)
                break
            else:
                break
        t = elem
        for n in reversed(dims):
            t = T('array', sub=t, n=n)
        return t


def size_of(t, dec):
    if t.kind == 'base':
        return FT_SIZE.get(t.name, 4)
    if t.kind in ('ptr', 'func'):
        return 4
    if t.kind in ('const', 'volatile'):
        return size_of(t.sub, dec)
    if t.kind == 'array':
        return t.n * size_of(t.sub, dec)
    if t.kind in ('struct', 'union'):
        return t.die.a.get(AT_BYTESIZE, 0)
    if t.kind == 'enum':
        return t.die.a.get(AT_BYTESIZE, 4)
    return 4


def member_offset(a):
    loc = a.get(AT_LOCATION, b'')
    # OP_CONST(4) <u32> OP_ADD(7)
    if len(loc) >= 5 and loc[0] == 4:
        return struct.unpack_from('>I', loc, 1)[0]
    return 0


def reg_of(a):
    loc = a.get(AT_LOCATION, b'')
    if len(loc) >= 5 and loc[0] == 1:
        r = struct.unpack_from('>I', loc, 1)[0]
        return f"r{r}" if r < 32 else f"f{r - 32}"
    if len(loc) >= 5 and loc[0] == 2:
        return 'stack'
    return None


def enum_items(d):
    b = d.a.get(AT_ELEMLIST, b'')
    items, i = [], 0
    while i + 4 < len(b):
        v = struct.unpack_from('>i', b, i)[0]; i += 4
        e = b.index(0, i); items.append((b[i:e].decode('latin1'), v)); i = e + 1
    return list(reversed(items))


# ------------------------------------------------------------ canonical keys
class Registry:
    """Deduplicate struct/union/enum DIEs by layout; give each a C name."""

    def __init__(self, dec):
        self.dec = dec
        self.key_of = {}      # die offset -> key
        self.name_of = {}     # key -> C name
        self.rep = {}         # key -> representative T
        self.order = []       # keys in first-seen order

    def shallow(self, t):
        d = t.die
        if t.kind == 'enum':
            return ('enum', t.name, tuple(enum_items(d)))
        mem = tuple((c.a.get(AT_NAME), member_offset(c.a)) for c in d.children if c.tag == 'member')
        return (t.kind, t.name, d.a.get(AT_BYTESIZE, 0), mem)

    def key(self, t, depth=0):
        """Structural key; pointed-to aggregates use their shallow key to avoid cycles."""
        if t.die.off in self.key_of:
            return self.key_of[t.die.off]
        if t.kind == 'enum':
            k = self.shallow(t)
        else:
            mem = []
            for c in t.die.children:
                if c.tag != 'member':
                    continue
                mem.append((c.a.get(AT_NAME), member_offset(c.a), c.a.get(AT_BITSIZE), c.a.get(AT_BITOFF),
                            self.tkey(self.dec.type_of(c.a), depth + 1)))
            k = (t.kind, t.name, t.die.a.get(AT_BYTESIZE, 0), tuple(mem))
        self.key_of[t.die.off] = k
        if k not in self.rep:
            self.rep[k] = t
            self.order.append(k)
        return k

    def tkey(self, t, depth):
        if t.kind == 'base':
            return t.name
        if t.kind in ('const', 'volatile'):
            return (t.kind, self.tkey(t.sub, depth))
        if t.kind == 'ptr':
            s = t.sub
            while s.kind in ('const', 'volatile'):
                s = s.sub
            if s.kind in ('struct', 'union', 'enum'):
                return ('ptr', self.shallow(s), self.tkey(t.sub, depth) if t.sub.kind in ('const', 'volatile') else None)
            return ('ptr', self.tkey(t.sub, depth))
        if t.kind == 'array':
            return ('arr', t.n, self.tkey(t.sub, depth))
        if t.kind == 'func':
            return ('fn', self.tkey(t.sub, depth), tuple(self.tkey(p, depth) for p in t.params), t.varargs)
        if t.kind in ('struct', 'union', 'enum'):
            return self.key(t, depth)
        return 'int'

    def assign_names(self, type_names=None):
        """type_names: {synthetic name: chosen name} from overrides.json "type_names"; applied after
        the synthetic name (including any _2/_3 layout suffix) is fixed, so the keys stay stable."""
        type_names = type_names or {}
        used = collections.Counter()
        shallow_name = {}
        for k in self.order:
            t = self.rep[k]
            if t.name:
                base = re.sub(r'\W', '_', t.name)
                if base.startswith('@'):
                    base = 'anon'
            else:
                if t.kind == 'enum':
                    items = enum_items(t.die)
                    base = 'enum_' + (items[0][0] if items else 'empty')
                else:
                    mem = [c.a.get(AT_NAME) or 'x' for c in t.die.children if c.tag == 'member'][:2]
                    base = f"anon_{'_'.join(mem) or 'empty'}_{t.die.a.get(AT_BYTESIZE, 0):x}"
            if re.match(r'^@?enum\$', base) or base.startswith('_enum_'):
                items = enum_items(t.die)
                base = 'enum_' + (items[0][0] if items else 'empty')
            used[base] += 1
            name = base if used[base] == 1 else f"{base}_{used[base]}"
            self.name_of[k] = type_names.get(name, name)
        # map shallow keys (used by pointers) to a name: the first full key with that shallow form
        self.shallow_name = {}
        for k in self.order:
            sk = self.shallow(self.rep[k])
            self.shallow_name.setdefault(sk, self.name_of[k])


# ------------------------------------------------------------- C rendering
class Renderer:
    def __init__(self, reg):
        self.reg = reg
        self.dec = reg.dec

    def agg_name(self, t):
        if t.die.off in self.reg.key_of:
            return self.reg.name_of[self.reg.key_of[t.die.off]]
        return self.reg.shallow_name.get(self.reg.shallow(t), 'UNKNOWN')

    def decl(self, t, name=''):
        """C declaration of `name` with type t."""
        if t.kind == 'base':
            return f"{t.name} {name}".rstrip() if name else t.name
        if t.kind in ('struct', 'union', 'enum'):
            return f"{self.agg_name(t)} {name}".rstrip()
        if t.kind in ('const', 'volatile'):
            inner = t.sub
            if inner.kind in ('base', 'struct', 'union', 'enum'):
                return f"{t.kind} {self.decl(inner, name)}"
            return self.decl(inner, f"{t.kind} {name}".strip())
        if t.kind == 'ptr':
            sub = t.sub
            if sub.kind in ('array', 'func'):
                return self.decl(sub, f"(*{name})")
            return self.decl(sub, f"*{name}")
        if t.kind == 'array':
            return self.decl(t.sub, f"{name}[{t.n if t.n > 0 else ''}]")
        if t.kind == 'func':
            ps = [self.decl(p) for p in t.params]
            if t.varargs:
                ps.append('...')
            return self.decl(t.sub, f"{name}({', '.join(ps) if ps else 'void'})")
        return f"int {name}"

    def deps(self, k):
        """Aggregate keys this aggregate needs complete (by-value members)."""
        t = self.reg.rep[k]
        out = []
        if t.kind == 'enum':
            return out
        for c in t.die.children:
            if c.tag != 'member':
                continue
            mt = self.dec.type_of(c.a)
            while mt.kind in ('array', 'const', 'volatile'):
                mt = mt.sub
            if mt.kind in ('struct', 'union'):
                out.append(self.reg.key(mt))
        return out

    vol_members = {}

    def body(self, k):
        t = self.reg.rep[k]
        name = self.reg.name_of[k]
        vols = self.vol_members.get(name, set())
        size = t.die.a.get(AT_BYTESIZE, 0)
        if t.kind == 'enum':
            return name, f"/* size {size:#x} */\ntypedef int {name};"
        lines = [f"/* size {size:#x} */", f"typedef {t.kind} {name} {{"]
        for c in t.die.children:
            if c.tag != 'member':
                continue
            mt = self.dec.type_of(c.a)
            mn = c.a.get(AT_NAME) or f"_pad{member_offset(c.a):x}"
            bits = c.a.get(AT_BITSIZE)
            d = self.decl(mt, mn) + (f" : {bits}" if bits else '')
            if mn in vols:
                d = 'volatile ' + d
            lines.append(f"    {d}; /* {member_offset(c.a):#x} */")
        lines.append(f"}} {name};")
        return name, '\n'.join(lines)


def main():
    elf_path, out_dir = sys.argv[1], sys.argv[2]
    ov_path = os.path.join(out_dir, 'overrides.json')
    ov = json.load(open(ov_path)) if os.path.exists(ov_path) else {}
    vol_vars = set(ov.get('volatile_variables', []))
    const_vars = set(ov.get('const_variables', []))
    type_names = ov.get('type_names', {})
    vol_members = {type_names.get(k, k): set(v) for k, v in ov.get('volatile_members', {}).items()}
    proto_over = ov.get('prototypes', {})
    extern_over = ov.get('extern_declarations', {})
    e = Elf(elf_path)
    dbg = e.data('.debug')
    dies, order = read_dies(dbg)
    dec = Decoder(dies)
    reg = Registry(dec)
    for d in order:
        if d.tag in ('struct', 'union', 'enum') and d.tag != 'padding':
            reg.key(dec.udt(d.off))
    reg.assign_names(type_names)
    ren = Renderer(reg)
    ren.vol_members = vol_members

    # ---- rush_types.h: forward typedefs, enum constants, then definitions in dependency order
    out = ['/* Generated by tools/dwarftypes.py from rush_d.elf DWARF v1. Do not edit by hand;',
           '   rename anonymous types through "type_names" in overrides.json and regenerate. */',
           '#ifndef RUSH_TYPES_H', '#define RUSH_TYPES_H', '',
           '#ifndef NULL', '#define NULL 0', '#endif', '']
    for k in reg.order:
        t = reg.rep[k]
        if t.kind in ('struct', 'union'):
            out.append(f"typedef {t.kind} {reg.name_of[k]} {reg.name_of[k]};")
    out.append('')
    seen_enum = set()
    for k in reg.order:
        t = reg.rep[k]
        if t.kind != 'enum':
            continue
        items = [(n, v) for n, v in enum_items(t.die) if n not in seen_enum]
        seen_enum.update(n for n, _ in items)
        out.append(f"typedef int {reg.name_of[k]};")
        if items:
            out.append('enum { ' + ', '.join(f"{n} = {v}" for n, v in items) + ' };')
    out.append('')
    done, emitted = set(), []

    def emit(k, stack=()):
        if k in done or k in stack:
            return
        for dk in ren.deps(k):
            emit(dk, stack + (k,))
        done.add(k)
        if reg.rep[k].kind != 'enum':
            emitted.append(ren.body(k)[1])
    for k in reg.order:
        emit(k)
    out.extend(x + '\n' for x in emitted)
    out.append('#endif')
    open(os.path.join(out_dir, 'rush_types.h'), 'w', encoding='utf-8', newline='\n').write('\n'.join(out))

    # ---- per-CU declarations, and global prototypes/externs
    cus = collections.OrderedDict()
    protos, externs = {}, {}
    for d in order:
        if d.tag == 'cu':
            cus[d.off] = {'path': d.a.get(AT_NAME, ''), 'functions': [], 'variables': []}
    for d in order:
        cu = d.cu
        if cu is None or d.tag not in ('gfunc', 'sfunc', 'gvar', 'lvar'):
            continue
        parent_is_cu = any(d is c for c in cu.children) or True
        if d.tag in ('gfunc', 'sfunc'):
            ret = dec.type_of(d.a)
            params = [c for c in d.children if c.tag == 'param']
            ps = [ren.decl(dec.type_of(c.a), c.a.get(AT_NAME) or f"arg{i}") for i, c in enumerate(params)]
            if any(c.tag == 'varargs' for c in d.children):
                ps.append('...')
            name = d.a.get(AT_NAME, '')
            sig = ren.decl(T('func', sub=ret, params=[]), '')  # placeholder for return type only
            ret_txt = ren.decl(ret, '').strip()
            text = f"{ren.decl(ret, name + '(' + (', '.join(ps) if ps else 'void') + ')')}"
            if ret.kind == 'ptr' and ret.sub.kind in ('func', 'array'):
                text = ren.decl(T('func', sub=ret, params=[dec.type_of(c.a) for c in params]), name)
            locals_ = []
            for c in _walk(d):
                if c.tag == 'lvar' and c is not d:
                    loc = c.a.get(AT_LOCATION, b'')
                    static_addr = struct.unpack_from('>I', loc, 1)[0] if len(loc) >= 5 and loc[0] == 3 else None
                    locals_.append({'decl': ren.decl(dec.type_of(c.a), c.a.get(AT_NAME, '')),
                                    'reg': reg_of(c.a), 'static_addr': static_addr})
            fn = {'name': name, 'static': d.tag == 'sfunc', 'decl': text, 'lo': d.a.get(AT_LOWPC, 0),
                  'hi': d.a.get(AT_HIGHPC, 0), 'stripped': not d.a.get(AT_LOWPC),
                  'params': [{'decl': ren.decl(dec.type_of(c.a), c.a.get(AT_NAME) or f"arg{i}"), 'reg': reg_of(c.a)}
                             for i, c in enumerate(params)],
                  'locals': locals_}
            cus[cu.off]['functions'].append(fn)
            if d.tag == 'gfunc' and name:
                # DWARF v1 from CodeWarrior has no AT_prototyped, so an empty parameter list may be
                # an unprototyped definition that callers pass arguments to: declare it as f().
                if name in proto_over:  # the original's header declaration (see overrides.json)
                    protos[name] = proto_over[name]
                    continue
                protos.setdefault(name, text.replace(name + '(void)', name + '()') if not params else text)
        elif d.tag in ('gvar', 'lvar'):
            # only file-scope variables (direct children of the CU)
            if not any(d is c for c in cu.children):
                continue
            loc = d.a.get(AT_LOCATION, b'')
            addr = struct.unpack_from('>I', loc, 1)[0] if len(loc) >= 5 and loc[0] == 3 else None
            name = d.a.get(AT_NAME, '')
            text = ren.decl(dec.type_of(d.a), name)
            if name in vol_vars:
                text = 'volatile ' + text
            if name in const_vars:
                text = 'const ' + text
            cus[cu.off]['variables'].append({'name': name, 'static': d.tag == 'lvar', 'decl': text, 'addr': addr,
                                             'size': size_of(dec.type_of(d.a), dec)})
            if d.tag == 'gvar' and name and addr:
                # prefer the declaration with a complete size (the definition) over `extern T x[]`
                sz = size_of(dec.type_of(d.a), dec)
                if name not in externs or sz > externs[name][1]:
                    externs[name] = (text, sz)
    p = ['/* Generated by tools/dwarftypes.py: prototypes of every global function and extern',
         '   declarations of every global variable in rush_d.elf. */',
         '#ifndef RUSH_PROTOS_H', '#define RUSH_PROTOS_H', '', '#include "rush/rush_types.h"',
         '#include "rush/libc.h"', '#include "rush/dolphin.h"', '']
    for n in sorted(protos):
        p.append(protos[n] + ';')
    p.append('')
    for n in sorted(externs):
        p.append('extern ' + extern_over.get(n, externs[n][0]) + ';')
    p += ['', '#endif']
    open(os.path.join(out_dir, 'rush_protos.h'), 'w', encoding='utf-8', newline='\n').write('\n'.join(p) + '\n')
    json.dump({v['path']: v for v in cus.values()}, open(os.path.join(out_dir, 'rush_decls.json'), 'w'), indent=1)
    print(f"types: {len(reg.order)} unique aggregates/enums from {sum(1 for d in order if d.tag in ('struct', 'union', 'enum'))} DIEs")
    print(f"prototypes: {len(protos)}  externs: {len(externs)}  compile units: {len(cus)}")


def _walk(d):
    for c in d.children:
        yield c
        yield from _walk(c)


if __name__ == '__main__':
    main()
