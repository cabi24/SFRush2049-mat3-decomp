"""Try every order of a run of consecutive statements (or declarations) in one function.

usage: python tools/permute.py <src.c> <func> <text of the first line> <number of lines> [--max N]

The lines starting at the first line containing <text> are reordered in every possible way
(up to --max permutations, default 720), each variant is compiled with tools/cwmatch.py, and
the best results are printed. Lines must be self-contained statements; keep the count small
(6 lines = 720 compiles). The variant is written to ../work/_v.c.
"""
import itertools
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "work", "_v.c")


def score(out):
    if "MATCH" in out:
        return 0
    m = re.search(r"(\d+)/(\d+) instrs differ", out)
    if m:
        return int(m.group(1))
    return 10 ** 6


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--max")]
    maxn = 720
    for a in sys.argv[1:]:
        if a.startswith("--max"):
            maxn = int(a.split("=", 1)[1])
    src, func, first, count = args[0], args[1], args[2], int(args[3])
    lines = open(src, encoding="utf-8").read().split("\n")
    start = next(i for i, l in enumerate(lines) if first in l)
    block = lines[start:start + count]
    results = []
    for n, perm in enumerate(itertools.permutations(range(count))):
        if n >= maxn:
            break
        new = lines[:start] + [block[i] for i in perm] + lines[start + count:]
        open(OUT, "w", encoding="utf-8").write("\n".join(new))
        r = subprocess.run([sys.executable, os.path.join(HERE, "cwmatch.py"), OUT, func, "--versions", "2.6"],
                           capture_output=True, text=True)
        out = r.stdout.strip().splitlines()[-1] if r.stdout.strip() else "error"
        results.append((score(out), perm, out.split("]")[-1].strip()))
        if score(out) == 0:
            break
    results.sort()
    for sc, perm, out in results[:8]:
        print(f"{out:40s} order {perm}")
    best = results[0][1]
    print("best order:")
    for i in best:
        print("   ", block[i].strip())


if __name__ == "__main__":
    main()
