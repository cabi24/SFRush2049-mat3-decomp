"""Resolve byte offsets inside a type from include/rush/rush_types.h to member paths.

usage: python tools/fields.py <type> <offset hex> [<offset hex> ...]
       python tools/fields.py <type>            (print the whole layout, flattened)
"""
import re
import sys

PRIM = {"char": 1, "signed char": 1, "unsigned char": 1, "short": 2, "signed short": 2,
        "unsigned short": 2, "int": 4, "unsigned int": 4, "long": 4, "unsigned long": 4,
        "signed long": 4, "float": 4, "double": 8, "long long": 8, "unsigned long long": 8}


def load():
    src = open("include/rush/rush_types.h", encoding="utf-8").read()
    types = {}
    for m in re.finditer(r"/\* size (0x[0-9a-f]+) \*/\ntypedef (struct|union) (\w+) \{(.*?)\n\} (\w+);", src, re.S):
        size, _, _, body, name = m.groups()
        members = []
        for line in body.split("\n"):
            mm = re.match(r"\s*(.+?)\s*(\**)\s*(\w+)((?:\[\d+\])*);\s*/\* (0x[0-9a-f]+) \*/", line)
            if not mm:
                mm2 = re.match(r"\s*(.+?)\s*\(\*(\w+)\)\(.*\);\s*/\* (0x[0-9a-f]+) \*/", line)
                if mm2:
                    members.append((mm2.group(2), "fnptr", "*", [], int(mm2.group(3), 16)))
                continue
            t, ptr, n, dims, off = mm.groups()
            members.append((n, t, ptr, [int(d) for d in re.findall(r"\d+", dims)], int(off, 16)))
        types[name] = (int(size, 16), members)
    for m in re.finditer(r"typedef int (\w+);", src):
        types.setdefault(m.group(1), (4, None))
    return types


def tsize(types, t, ptr):
    if ptr:
        return 4
    if t in PRIM:
        return PRIM[t]
    if t in types:
        return types[t][0]
    return 4


def resolve(types, t, off, path):
    size, members = types.get(t, (0, None))
    if not members:
        return path + (f" (+{off:#x})" if off else "")
    best = None
    for n, mt, ptr, dims, moff in members:
        es = tsize(types, mt, ptr)
        count = 1
        for d in dims:
            count *= d
        if moff <= off < moff + es * count:
            best = (n, mt, ptr, dims, moff, es)
    if best is None:
        return path + f" (+{off:#x}: padding?)"
    n, mt, ptr, dims, moff, es = best
    rel = off - moff
    p = f"{path}.{n}" if path else n
    if dims:
        idx, rel = divmod(rel, es)
        # flatten multi-dim index
        sub = []
        for d in reversed(dims[1:]):
            idx, r = divmod(idx, d) if False else (idx, None)
        p += f"[{idx}]" if len(dims) == 1 else f"[flat {idx}]"
    if ptr or mt not in types:
        return p + (f" (+{rel:#x})" if rel else "")
    return resolve(types, mt, rel, p)


def main():
    types = load()
    t = sys.argv[1]
    if t not in types:
        sys.exit(f"unknown type {t}")
    if len(sys.argv) == 2:
        for n, mt, ptr, dims, moff in types[t][1]:
            print(f"{moff:#6x}  {mt}{' ' + ptr if ptr else ''} {n}{''.join(f'[{d}]' for d in dims)}")
        return
    for o in sys.argv[2:]:
        off = int(o, 16)
        print(f"{off:#6x}  {resolve(types, t, off, '')}")


if __name__ == "__main__":
    main()
