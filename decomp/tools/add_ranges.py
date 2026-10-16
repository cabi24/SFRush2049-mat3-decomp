"""Add section ranges to units in splits.txt, keeping each unit's sections in canonical order.

usage: python tools/add_ranges.py config/2049LE/splits.txt "<unit>|<section>|<start>|<end>" ...
"""
import sys

ORDER = ['.init', '.text', '.ctors', '.dtors', '.rodata', '.data', '.bss', '.sdata', '.sbss', '.sdata2', '.sbss2']
path, specs = sys.argv[1], sys.argv[2:]
lines = open(path, encoding='utf-8').read().split('\n')
for spec in specs:
    unit, sec, start, end = spec.split('|')
    i = lines.index(unit + ':')
    j = i + 1
    while j < len(lines) and lines[j].startswith('\t'):
        j += 1
    body = lines[i + 1:j]
    assert not any(l.split()[0] == sec for l in body), f"{unit} already has {sec}"
    body.append(f"\t{sec:<11} start:{start} end:{end}")
    body.sort(key=lambda l: ORDER.index(l.split()[0]))
    lines[i + 1:j] = body
    print(f"{unit}: added {sec} {start}-{end}")
open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(lines))
