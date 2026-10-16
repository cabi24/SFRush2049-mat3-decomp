"""Drop zero-length section ranges (and then-empty units) from a dtk splits.txt.

`dtk elf config` emits entries like `.sbss start:0x802A4860 end:0x802A4860` for files that
reference a section without contributing bytes; `dtk dol split` rejects them.
usage: python tools/fix_splits.py config/2049LE/splits.txt
"""
import re
import sys

path = sys.argv[1]
lines = open(path, encoding='utf-8').read().splitlines()
out, unit, body, dropped = [], None, [], 0


def flush():
    if unit is not None and body:
        out.append(unit)
        out.extend(body)
        out.append('')


rng = re.compile(r'start:(0x[0-9A-Fa-f]+)\s+end:(0x[0-9A-Fa-f]+)')
in_sections = True
for ln in lines:
    if in_sections:
        out.append(ln)
        if ln == '':
            in_sections = False
        continue
    if ln and not ln[0].isspace():
        flush()
        unit, body = ln, []
    elif ln.strip():
        m = rng.search(ln)
        if m and int(m.group(1), 16) == int(m.group(2), 16):
            dropped += 1
            continue
        body.append(ln)
flush()
open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(out).rstrip('\n') + '\n')
print(f"dropped {dropped} empty ranges")
