"""Rename splits.txt units for the game's own sources to their original relative paths.

Units are matched to DWARF compile units by exact (case-sensitive) basename, using
rush_d_compile_units.csv; D:\\rush_ngc\\Rush_NGC\\ is stripped, so e.g. `os.c` becomes
`GameSource/src/os.c`. This keeps the game's os.c apart from the SDK's OS.c, which
otherwise collide as obj/os.o on case-insensitive filesystems.
usage: python tools/place_units.py config/2049LE/splits.txt ../rush_d_compile_units.csv
"""
import csv
import collections
import sys

splits, cu_csv = sys.argv[1], sys.argv[2]
BS = chr(92)
prefix = 'D:' + BS + 'rush_ngc' + BS + 'Rush_NGC' + BS
by_base = {}
for row in csv.DictReader(open(cu_csv, encoding='utf-8')):
    p = row['source_path']
    if p.startswith(prefix):
        by_base.setdefault(row['source_file'], set()).add(p[len(prefix):].replace(BS, '/'))

lines = open(splits, encoding='utf-8').read().split('\n')
header, renamed = True, 0
for i, ln in enumerate(lines):
    if header:
        header = ln != ''
        continue
    if ln and not ln[0].isspace() and ln.endswith(':'):
        unit = ln[:-1]
        target = by_base.get(unit)
        if target and len(target) == 1:
            lines[i] = next(iter(target)) + ':'
            renamed += 1
open(splits, 'w', encoding='utf-8', newline='\n').write('\n'.join(lines))

units = [ln[:-1] for ln in lines if ln and not ln[0].isspace() and ln.endswith(':') and ln != 'Sections:']
stems = collections.defaultdict(list)
for u in units:
    stems[u.rsplit('.', 1)[0].lower()].append(u)
clashes = {k: v for k, v in stems.items() if len(v) > 1}
print(f"renamed {renamed} units; case-insensitive object clashes left: {clashes or 'none'}")
