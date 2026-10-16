"""Set an object's status in configure.py.

usage: python tools/mark.py <Matching|NonMatching|Equivalent> GameSource/src/os.c [more.c ...]
"""
import re
import sys

status, units = sys.argv[1], sys.argv[2:]
assert status in ('Matching', 'NonMatching', 'Equivalent')
p = 'configure.py'
s = open(p, encoding='utf-8').read()
for u in units:
    pat = re.compile(r'Object\((?:Matching|NonMatching|Equivalent), "' + re.escape(u) + r'"')
    s, n = pat.subn(f'Object({status}, "{u}"', s)
    print(f"{u}: {'set to ' + status if n else 'NOT FOUND'}")
open(p, 'w', encoding='utf-8').write(s)
