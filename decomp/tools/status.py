"""Summarise decompilation progress from build/2049LE/report.json (run `ninja build/2049LE/report.json` first).

usage: python tools/status.py [--md out.md]
Shows overall game-code progress, units fully matched, units in progress, and near-miss
functions (>= 90% but < 100%) worth another pass.
"""
import json
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')


def main():
    rep = json.load(open(os.path.join(ROOT, 'build', '2049LE', 'report.json')))
    cfg = open(os.path.join(ROOT, 'configure.py'), encoding='utf-8').read()
    linked = set(re.findall(r'Object\(Matching, "([^"]+)"', cfg))
    game = [u for u in rep['units'] if u['name'].replace('rush_d/', '').startswith(('GameSource/', 'NGC/'))]
    tot_code = sum(int(u['measures'].get('total_code', 0)) for u in game)
    mat_code = sum(int(u['measures'].get('matched_code', 0)) for u in game)
    tot_fn = sum(int(u['measures'].get('total_functions', 0)) for u in game)
    mat_fn = sum(int(u['measures'].get('matched_functions', 0)) for u in game)
    lines = [f"Game code (GameSource + NGC): {mat_code:,}/{tot_code:,} bytes ({100 * mat_code / tot_code:.2f}%), "
             f"{mat_fn}/{tot_fn} functions, {len(linked)} of {len(game)} files linked from C (Matching)", '']
    done, partial, near = [], [], []
    for u in sorted(game, key=lambda u: u['name']):
        n = u['name'].replace('rush_d/', '')
        m = u['measures']
        src = next((x for x in linked if x.rsplit('.', 1)[0] == n), None)
        if src:
            done.append(n)
            continue
        fns = u.get('functions', [])
        if int(m.get('matched_functions', 0)) or any((f.get('fuzzy_match_percent') or 0) > 0 for f in fns):
            partial.append(f"{n}: {m.get('matched_functions', 0)}/{m.get('total_functions', 0)} functions, "
                           f"code {float(m.get('matched_code_percent', 0)):.1f}%")
        for f in fns:
            p = f.get('fuzzy_match_percent') or 0
            if 90 <= p < 100:
                near.append(f"{n} {f['name']}: {p:.1f}% ({f.get('size')} bytes)")
    lines.append(f"## Linked from C ({len(done)})")
    lines += ['- ' + d for d in done] + ['']
    lines.append(f"## In progress ({len(partial)})")
    lines += ['- ' + p for p in partial] + ['']
    lines.append(f"## Near misses ({len(near)})")
    lines += ['- ' + x for x in near]
    text = '\n'.join(lines) + '\n'
    if '--md' in sys.argv:
        open(sys.argv[sys.argv.index('--md') + 1], 'w', encoding='utf-8').write(text)
    print(text)


if __name__ == '__main__':
    main()
