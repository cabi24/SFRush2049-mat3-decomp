"""Reorder a TU's function definitions into the original (.text) order.

usage (from decomp/): python tools/reorder.py <unit> <csv source_file> [attach.json]
  attach.json: {"<regex on a file-scope static/data declaration>": "<function it must precede>"}
Non-function top-level items keep their relative order and go first, except declarations matched
by attach.json, which are emitted right before the named function. Inline helpers (functions not
in the original list) are emitted after the preamble. Prototypes for every original function are
inserted before the first definition. Line endings are preserved byte-wise.
"""
import csv, json, os, re, sys

unit, csvname = sys.argv[1], sys.argv[2]
attach_to = json.load(open(sys.argv[3])) if len(sys.argv) > 3 else {}
p = f'src/{unit}.c'
b = open(p, 'rb').read()
lines = b.split(b'\n')
NAME = re.compile(rb'^(?:static\s+|inline\s+)*[A-Za-z_][\w\s\*]*?\b(\w+)\s*\(')
items = []; k = 0
while k < len(lines):
    s = lines[k].rstrip(b'\r')
    if not s.strip(): k += 1; continue
    if s.startswith(b'/*'):
        j = k
        while b'*/' not in lines[j]: j += 1
        items.append(('other', k, j, s[:40])); k = j + 1; continue
    if s.startswith(b'#') or (b'{' in s and s.count(b'{') == s.count(b'}') and not s.endswith(b';')):
        items.append(('other', k, k, s[:40])); k += 1; continue
    m = NAME.match(s)
    if m and b'=' not in s.split(b'(')[0] and not s.startswith((b' ', b'\t')):
        j = k; sig = b''
        while True:
            sig += lines[j].rstrip(b'\r') + b' '
            if sig.count(b'(') == sig.count(b')'): break
            j += 1
        sig = sig.strip()
        if sig.endswith(b'{') or (j + 1 < len(lines) and lines[j + 1].rstrip(b'\r') == b'{'):
            body = j if sig.endswith(b'{') else j + 1
            depth = 0
            while True:
                depth += lines[body].count(b'{') - lines[body].count(b'}')
                if depth == 0: break
                body += 1
            start = k
            while start > 0 and lines[start - 1].rstrip(b'\r').startswith((b'/* 0x', b'#pragma force_active on')): start -= 1
            end = body
            if end + 1 < len(lines) and lines[end + 1].rstrip(b'\r').startswith(b'#pragma force_active reset'): end += 1
            items.append(('func', start, end, m.group(1).decode())); k = end + 1; continue
    j = k; depth = 0
    while True:
        depth += lines[j].count(b'{') - lines[j].count(b'}')
        if depth <= 0 and lines[j].rstrip(b'\r').endswith(b';'): break
        j += 1
    items.append(('decl', k, j, s[:50])); k = j + 1
funcs = [it for it in items if it[0] == 'func']
order = [(int(r['address'], 16), r['name']) for r in csv.DictReader(open('../rush_d_functions.csv', encoding='utf-8', errors='replace'))
         if r['source_file'] == csvname and r['stripped'] != '1' and r['address'] != '0x00000000']
order.sort(); names = [n for _, n in order]
have = {f[3]: f for f in funcs}
missing = [n for n in names if n not in have]
if missing:
    sys.exit(f'missing function definitions: {missing}')
pre, attach = [], {}
for it in items:
    if it[0] == 'func': continue
    text = b'\n'.join(lines[it[1]:it[2] + 1]); tgt = None
    for pat, f in attach_to.items():
        if re.search(pat.encode(), text): tgt = f
    (attach.setdefault(tgt, []) if tgt else pre).append(it)
helpers = [f for f in funcs if f[3] not in names]
nl = b'\r\n' if b.count(b'\r\n') > b.count(b'\n') // 2 else b'\n'
sigs = []
for n in names:
    f = have[n]
    sig = b''
    for ln in lines[f[1]:f[2] + 1]:
        t = ln.rstrip(b'\r')
        if t.startswith((b'/*', b'#')): continue
        sig += t + b' '
        if sig.count(b'(') == sig.count(b')') and b'(' in sig: break
    sig = re.sub(rb'\s+', b' ', sig).strip().rstrip(b'{').strip()
    sig = re.sub(rb'\(\s*void\s*\)$', b'()', sig)   # unprototyped, as the shared header declares them; callers may pass extra args
    sigs.append(sig + b';')
out = []
def emit(it): out.extend(lines[it[1]:it[2] + 1]); out.append(b'')
for it in pre: emit(it)
for f in helpers: emit(f)
out.append(b'/* prototypes: the functions are defined in their original order below */')
out.extend(sigs); out.append(b'')
for n in names:
    for it in attach.get(n, []): emit(it)
    emit(have[n])
res = b'\n'.join(out).rstrip(b'\n') + b'\n'
if nl == b'\r\n':
    res = res.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
open(p, 'wb').write(res)
print(f'{unit}: {len(names)} originals, helpers {[f[3] for f in helpers]}, attached {{{", ".join(f"{k}: {len(v)}" for k, v in attach.items())}}}')
