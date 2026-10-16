"""Generate a C skeleton for a unit: shared header, data with initializers, function stubs.

usage: python tools/skel.py <unit> [--data-only] > src/<unit>.c

Uses include/rush/rush_decls.json (from tools/dwarftypes.py) for declarations in the shared
type names, and the unit2c layout logic for initializer values from the original bytes.
Functions come in address order with signature, locals (register noted) and a TODO body;
the original assembly is in build/2049LE/asm/<unit>.s. Compiler-generated objects (@NNN
literals, gap_ padding) are not emitted: they come from the code once it is written.
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ctx import asm_text, dwarf_section  # noqa: E402
from data2c import parse_asm  # noqa: E402
import unit2c  # noqa: E402

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
BS = chr(92)


def cu_for(unit, decls):
    want = unit.replace('/', BS)
    for path, cu in decls.items():
        if path.rsplit('.', 1)[0].endswith(BS + want):
            return cu
    return None


def initializer_values(unit):
    """name -> list of C value strings, using unit2c's layout of the dtk text."""
    text = dwarf_section(unit) or ''
    objs = parse_asm(asm_text(unit))
    strings = {n: o[1] for n, o in objs.items() if n.startswith('@') and o[1] and o[1].endswith(b'\0')
               and all(32 <= c < 127 or c in (9, 10) for c in o[1].rstrip(b'\0'))}
    named, vars_ = {}, []
    for st in unit2c.statements(text):
        v = unit2c.split_var(st)
        if v:
            vars_.append(v)
            continue
        m = re.match(r'^\s*(?://[^\n]*\n)*\s*((?:struct|union|enum) \w+) \{', st)
        if m:
            named[m.group(1)] = st.split('{', 1)[1].rsplit('}', 1)[0]
    lay = unit2c.Layout(named)
    local = {v[1] for v in vars_}
    out, externs = {}, set()
    for ttext, name, dims, size, addr in vars_:
        obj = objs.get(name)
        if not obj or obj[0] in ('.bss', '.sbss'):
            continue
        try:
            slots = lay.slots(ttext, unit2c.dims_of(dims))
            out[name] = [unit2c.value(obj[1], obj[2], off, kind, strings, externs, local) for off, kind in slots]
        except (KeyError, Exception) as ex:  # noqa: BLE001
            out[name] = None
    return out, objs


def wrap(vals):
    body, cur = [], ''
    for v in vals:
        if len(cur) + len(v) + 2 > 96:
            body.append(cur.rstrip())
            cur = ''
        cur += v + ', '
    body.append(cur.rstrip().rstrip(','))
    return '{\n    ' + '\n    '.join(body) + '\n}' if len(body) > 1 or len(body[0]) > 80 else '{ ' + body[0] + ' }'


def main():
    unit = sys.argv[1]
    data_only = '--data-only' in sys.argv
    decls = json.load(open(os.path.join(ROOT, 'include', 'rush', 'rush_decls.json')))
    cu = cu_for(unit, decls)
    if cu is None:
        sys.exit(f"no compile unit for {unit}")
    values, objs = initializer_values(unit)
    print(f"/* {unit.rsplit('/', 1)[-1]}.c - TODO: describe. Status: in progress.")
    print(f"   Original: {cu['path']}; assembly: build/2049LE/asm/{unit}.s */")
    print('#include "rush/rush_protos.h"')
    print()
    # data: definitions for variables placed in this unit
    defs = []
    for v in cu['variables']:
        name = v['name']
        if name not in objs or not v['addr']:
            continue
        sec = objs[name][0]
        static = 'static ' if v['static'] else ''
        const = 'const ' if sec in ('.rodata', '.sdata2') else ''
        if sec in ('.bss', '.sbss'):
            defs.append((sec, v['addr'], f"{static}{v['decl']};"))
        elif values.get(name) is None:
            defs.append((sec, v['addr'], f"/* TODO initializer: {static}{const}{v['decl']} ({sec}) */"))
        else:
            scalar = len(values[name]) == 1 and '[' not in v['decl'] and v['decl'].split()[0] in (
                'float', 'double', 'int', 'unsigned', 'signed', 'char', 'short', 'long', 'void')
            vals = values[name]
            if re.match(r'^(const )?unsigned char \*', v['decl']):
                # string literals are char *; the variable is unsigned char *
                vals = ['(unsigned char *)' + x if x.startswith('"') else x for x in vals]
            if not scalar and all(x in ('0', '0.0f', '0.0') for x in vals):
                init_txt = '{ 0 }'
            else:
                init_txt = vals[0] if scalar else wrap(vals)
            defs.append((sec, v['addr'], f"{static}{const}{v['decl']} = {init_txt};"))
    init = sorted([d for d in defs if d[0] not in ('.bss', '.sbss')], key=lambda d: d[1])
    unin = sorted([d for d in defs if d[0] in ('.bss', '.sbss')], key=lambda d: -d[1])
    live = sorted([f for f in cu['functions'] if not f['stripped']], key=lambda f: f['lo'])
    stripped = [f['name'] for f in cu['functions'] if f['stripped']]
    statics = [f for f in live if f['static']]
    if not data_only:
        # prototypes first: data initialisers may take a static function's address
        for f in statics:
            print(f"static {f['decl']};")
        if statics:
            print()
    for _, _, d in init + unin:
        print(d)
    if init or unin:
        print()
    if data_only:
        return
    if stripped:
        print("/* Stripped by the linker (string literals or array initialisers they used can remain in")
        print(f"   .rodata/.sdata2; see writeup): {', '.join(stripped)} */")
        print()
    for f in live:
        print(f"/* {f['lo']:#010x}, size {f['hi'] - f['lo']:#x} */")
        print(('static ' if f['static'] else '') + f['decl'])
        print('{')
        for l in f['locals']:
            if l['static_addr']:
                print(f"    /* static local at {l['static_addr']:#010x}: static {l['decl']}; */")
            else:
                print(f"    {l['decl']}; /* {l['reg'] or 'stack'} */")
        print('    /* TODO */')
        print('}')
        print()


if __name__ == '__main__':
    main()
