"""Replace `/* TODO */` bodies in a skeleton with bodies from a file.

usage: python tools/fill.py <unit> <bodies.txt>

bodies.txt holds sections headed `== <function>`; each section's text (verbatim, already
indented) replaces the `/* TODO */` line in that function of src/<unit>.c (or .cpp).
"""
import os
import re
import sys


def main():
    unit, bodies_path = sys.argv[1], sys.argv[2]
    path = None
    for ext in (".c", ".cpp"):
        if os.path.exists(os.path.join("src", unit + ext)):
            path = os.path.join("src", unit + ext)
    if path is None:
        sys.exit(f"no source for {unit}")
    bodies = {}
    cur = None
    for line in open(bodies_path, encoding="utf-8").read().split("\n"):
        m = re.match(r"^== (\w+)\s*$", line)
        if m:
            cur = m.group(1)
            bodies[cur] = []
        elif cur is not None:
            bodies[cur].append(line)
    bodies = {k: "\n".join(v).rstrip("\n") for k, v in bodies.items()}
    src = open(path, encoding="utf-8").read().split("\n")
    out, func, used = [], None, set()
    for line in src:
        m = re.match(r"^[A-Za-z_][\w\s\*]*?\b(\w+)\(", line)
        if m and not line.rstrip().endswith(";"):
            func = m.group(1)
        if line.strip() == "/* TODO */" and func in bodies:
            # keep a blank line between the local declarations and the body
            if bodies[func] and out and out[-1].strip() not in ("{", "") and out[-1].rstrip().endswith(("*/", ";")):
                out.append("")
            out.append(bodies[func])
            used.add(func)
            continue
        out.append(line)
    open(path, "w", encoding="utf-8", newline="\n").write("\n".join(out))
    missing = set(bodies) - used
    if missing:
        print("not filled (no TODO found):", ", ".join(sorted(missing)))
    print(f"{path}: filled {len(used)} function(s)")


if __name__ == "__main__":
    main()
