"""Try source variants of one function with the game's compiler settings.

usage: python tools/variants.py <src.c> <func> <old text> <new text> [<new text> ...]

Each <new text> replaces <old text> (which must occur in <src.c>) and the result is compiled
with tools/cwmatch.py against the original function. A literal backslash-n in an argument
becomes a newline. The variant is written to ../work/_v.c.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "work", "_v.c")


def main():
    src, func, old = sys.argv[1:4]
    s = open(src, encoding="utf-8").read()
    if old not in s:
        sys.exit("old text not found")
    for i, new in enumerate(sys.argv[4:]):
        open(OUT, "w", encoding="utf-8").write(s.replace(old, new.replace(chr(92) + "n", chr(10))))
        r = subprocess.run([sys.executable, os.path.join(HERE, "cwmatch.py"), OUT, func, "--versions", "2.6"],
                           capture_output=True, text=True)
        out = r.stdout.strip().splitlines()
        print(f"[{i}] {new[:90]!r}: {out[-1] if out else r.stderr[-300:]}")


if __name__ == "__main__":
    main()
