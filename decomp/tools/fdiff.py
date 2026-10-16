"""Side-by-side instruction diff of one function: our compiled object vs the original.

usage: python tools/fdiff.py <unit> <function> [--all]
Lines differing (ignoring relocated operands' resolved values) are marked '!'.
"""
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
DTK = os.path.join(ROOT, 'build', 'tools', 'dtk.exe')


def fn_lines(s_text, name):
    out, on = [], False
    for ln in s_text.splitlines():
        if ln.startswith('.fn ') and ln.split()[1].rstrip(',') == name:
            on = True
            continue
        if on and ln.startswith('.endfn'):
            break
        if on:
            m = re.match(r'^/\* [0-9A-F]{8} [0-9A-F]{8}  [0-9A-F ]{11} \*/\t(.*)$', ln)
            if m:
                out.append(norm(m.group(1)))
            elif ln.startswith('.L_'):
                out.append('LABEL')
    return out


def norm(ins):
    ins = re.sub(r'\.L_[0-9A-F]+', '.L', ins)       # branch label addresses
    ins = re.sub(r'"?@\d+"?', '@lit', ins)           # compiler literal numbering
    return ins.replace(' ', '')


def main():
    unit, func = sys.argv[1], sys.argv[2]
    def run(args):
        result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True,
                                stdin=subprocess.DEVNULL)
        if result.returncode:
            sys.exit(result.stdout + result.stderr)

    run([os.path.join(ROOT, 'build', 'tools', 'ninja.exe'), 'build/2049LE/src/' + unit + '.o'])
    # Native tools cannot reliably access the sandbox's redirected system TEMP.
    # Keep temporary output beside the objects, and remove it after reading.
    with tempfile.TemporaryDirectory(prefix='fdiff_', dir=os.path.join(ROOT, 'build')) as tmp:
        ours_s = os.path.join(tmp, 'ours.s')
        run([DTK, 'elf', 'disasm', os.path.join(ROOT, 'build/2049LE/src', unit + '.o'), ours_s])
        ours = fn_lines(open(ours_s, encoding='utf-8').read(), func)
    orig = fn_lines(open(os.path.join(ROOT, 'build/2049LE/asm', unit + '.s'), encoding='utf-8').read(), func)
    if not ours or not orig:
        sys.exit(f'Function {func!r} missing from compiled or original assembly')
    n = max(len(ours), len(orig))
    for i in range(n):
        a = ours[i] if i < len(ours) else ''
        b = orig[i] if i < len(orig) else ''
        mark = ' ' if a == b else '!'
        if mark == '!' or '--all' in sys.argv:
            print(f"{mark} {i:3}  {a:42} {b}")
    print(f"ours {len(ours)} lines, original {len(orig)} lines")


if __name__ == '__main__':
    main()
