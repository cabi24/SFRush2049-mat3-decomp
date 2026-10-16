"""Group CodeWarrior versions by the code they generate for each function in a probe file.

usage: python tools/cwprobe.py <probe.c> <versions,comma,separated> "<flags>"
Relocated fields are masked, so differences are real codegen differences.
"""
import hashlib
import os
import subprocess
import sys
import tempfile
import collections

sys.path.insert(0, os.path.dirname(__file__))
import cwmatch  # noqa: E402

src, versions, flags = sys.argv[1], sys.argv[2].split(','), sys.argv[3].split()
tmp = tempfile.mkdtemp(prefix='cwprobe_')
results = collections.defaultdict(dict)
for v in versions:
    obj = os.path.join(tmp, v + '.o')
    cmd = [os.path.join(cwmatch.COMPILERS, v, 'mwcceppc.exe'), *cwmatch.BASE, *flags, src, '-o', obj]
    r = subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=60)
    if r.returncode:
        print(v, 'failed:', (r.stdout + r.stderr).strip().splitlines()[-1:])
        continue
    funcs, rel = cwmatch.obj_funcs(obj)
    for name, (val, data) in funcs.items():
        words = [int.from_bytes(data[i:i + 4], 'big') for i in range(0, len(data), 4)]
        m = cwmatch.masked(words, rel, val)
        results[name][v] = hashlib.sha1(b''.join(w.to_bytes(4, 'big') for w in m)).hexdigest()[:8] + f"/{len(data):#x}"

split_points = collections.Counter()
for name in sorted(results):
    groups = collections.defaultdict(list)
    for v in versions:
        if v in results[name]:
            groups[results[name][v]].append(v)
    if len(groups) > 1:
        desc = ' | '.join(f"{','.join(vs)} ({h.split('/')[1]})" for h, vs in groups.items())
        print(f"{name:16} {desc}")
        order = [v for v in versions if v in results[name]]
        for a, b in zip(order, order[1:]):
            if results[name][a] != results[name][b]:
                split_points[f"{a} -> {b}"] += 1
same = [n for n in results if len(set(results[n].values())) == 1]
print(f"\nidentical across all versions: {len(same)} of {len(results)} functions")
print("where output changes between consecutive versions:", dict(split_points))
