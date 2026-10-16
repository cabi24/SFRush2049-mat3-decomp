"""Find which objects make an old mwldeppc hang, by linking subsets with a timeout.

Undefined-symbol errors are expected for partial links and count as "finished".
usage: python tools/bisect_link.py <GC version> [timeout_s]
"""
import os
import subprocess
import sys

ver = sys.argv[1]
limit = float(sys.argv[2]) if len(sys.argv) > 2 else 20
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
objs = [l.strip() for l in open(os.path.join(root, 'build/2049LE/rush_d.elf.rsp')) if l.strip()]
ld = os.path.join(root, 'build', 'compilers', 'GC', ver, 'mwldeppc.exe')
lcf = os.path.join(root, 'build', '2049LE', 'ldscript.lcf')
rsp = os.path.join(root, 'build', '2049LE', 'bisect.rsp')
out = os.path.join(root, 'build', '2049LE', 'bisect.elf')


def hangs(subset):
    open(rsp, 'w').write('\n'.join(subset) + '\n')
    try:
        subprocess.run([ld, '-fp', 'hardware', '-nodefaults', '-lcf', lcf, '-o', out, '@' + rsp], cwd=root,
                       stdin=subprocess.DEVNULL, capture_output=True, timeout=limit)
        return False
    except subprocess.TimeoutExpired:
        subprocess.run(['taskkill', '/F', '/IM', 'mwldeppc.exe'], capture_output=True)
        return True


# Shrink to a minimal hanging set: drop halves while the remainder still hangs.
cur = objs
assert hangs(cur), "full set does not hang"
chunk = len(cur) // 2
while chunk >= 1:
    i, changed = 0, False
    while i < len(cur):
        trial = cur[:i] + cur[i + chunk:]
        if trial and hangs(trial):
            cur, changed = trial, True
        else:
            i += chunk
    print(f"chunk {chunk}: {len(cur)} objects remain", flush=True)
    chunk = chunk // 2 if not changed or chunk > 1 else 0
print("minimal hanging set:")
for o in cur:
    print("  ", o)
