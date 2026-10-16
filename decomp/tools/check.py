"""Build one or more units' C sources and show objdiff's per-function and per-data scores.

usage: python tools/check.py GameSource/src/timer [more units...] --output-root <private-dir> [--all]
--all prints every item, otherwise only items below 100%.
The CLI uses isolated per-TU reports. --global-report explicitly requests the legacy full build/report.
"""
import json
import os
import subprocess
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
NINJA = os.path.join(ROOT, 'build', 'tools', 'ninja.exe')
# Local tool paths; binutils/sjiswrap are deliberately not installed (see writeup.md).
CONFIGURE_ARGS = ['--compilers', 'build/compilers', '--dtk', 'build/tools/dtk.exe',
                  '--objdiff', 'build/tools/objdiff-cli.exe', '--ninja', 'build/tools/ninja.exe',
                  '--binutils', 'build/binutils-not-installed', '--sjiswrap', 'build/tools/sjiswrap-not-installed.exe']


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    show_all = '--all' in sys.argv
    targets = [f"build/2049LE/src/{u}.o" for u in args]
    subprocess.run([sys.executable, 'configure.py', *CONFIGURE_ARGS], cwd=ROOT, capture_output=True,
                   stdin=subprocess.DEVNULL, timeout=300, check=True)
    r = subprocess.run([NINJA, *targets, 'build/2049LE/report.json'], cwd=ROOT, capture_output=True, text=True,
                       stdin=subprocess.DEVNULL, timeout=600)
    out = r.stdout + r.stderr
    if r.returncode:
        lines = [l for l in out.splitlines() if 'Alignment for' not in l and 'Missing configuration' not in l]
        print('\n'.join(lines[-30:]))
        sys.exit(1)
    with open(os.path.join(ROOT, 'build', '2049LE', 'report.json')) as report_file:
        rep = json.load(report_file)
    by = {u['name'].replace('rush_d/', ''): u for u in rep['units']}
    layout_ok = True
    for unit in args:
        u = by.get(unit)
        if not u:
            print(f"{unit}: not in report")
            layout_ok = False
            continue
        m = u.get('measures', {})
        print(f"== {unit}: code {float(m.get('matched_code_percent', 0)):.1f}% "
              f"({m.get('matched_code', 0)}/{m.get('total_code', 0)}), "
              f"data {float(m.get('matched_data_percent', 0)):.1f}% ({m.get('matched_data', 0)}/{m.get('total_data', 0)}), "
              f"functions {m.get('matched_functions', 0)}/{m.get('total_functions', 0)}")
        for kind in ('functions', 'sections'):
            for f in u.get(kind, []):
                pct = f.get('fuzzy_match_percent', 0) or 0
                if show_all or pct < 100:
                    print(f"   {kind[:-1]:8} {f['name']:40} {pct:6.1f}%  size={f.get('size')}")
        layout_ok = layout_check(unit) and layout_ok
    return 0 if layout_ok else 2


def layout_check(unit):
    """Fail on missing objects or incorrect target symbol placement, size or type."""
    from verify_layout import check_paths
    tgt_p = os.path.join(ROOT, 'build', '2049LE', 'obj', unit + '.o')
    our_p = os.path.join(ROOT, 'build', '2049LE', 'src', unit + '.o')
    return check_paths(tgt_p, our_p)


def isolated_main(argv=None):
    """CLI plumbing only; verification bodies remain owned by verify_layout."""
    import argparse
    from pathlib import Path
    import candidate

    supplied = list(sys.argv[1:] if argv is None else argv)
    if '--global-report' in supplied:
        # Keep main() as the legacy API for verifier tests and explicit global work.
        saved = sys.argv
        try:
            sys.argv = [saved[0], *[arg for arg in supplied if arg != '--global-report']]
            return main()
        finally:
            sys.argv = saved
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('units', nargs='+')
    parser.add_argument('--output-root', type=Path, required=True)
    parser.add_argument('--project', type=Path, default=Path(ROOT))
    parser.add_argument('--tool-root', type=Path)
    parser.add_argument('--timeout', type=int, default=60)
    parser.add_argument('--all', action='store_true')
    args = parser.parse_args(supplied)
    if len(args.units) > candidate.MAX_VARIANTS or not 1 <= args.timeout <= 300:
        parser.error('At most 16 TUs, timeout 1..300 seconds')
    try:
        from verify_layout import check_paths
    except ImportError:
        print('Focused check requires tools/verify_layout.py from the verification lane')
        return 2
    status = 0
    for unit in args.units:
        options = argparse.Namespace(unit=unit, project=args.project, tool_root=args.tool_root,
                                     output_root=args.output_root, candidate=None, function=None,
                                     max_variants=1, timeout=args.timeout)
        try:
            code, path = candidate.run(options)
        except (OSError, ValueError) as error:
            print(f'{unit}: {error}')
            status = max(status, 1)
            continue
        if code:
            status = max(status, code)
            if code == 130:
                break
            continue
        manifest = json.loads(path.read_text(encoding='utf-8'))
        work = Path(manifest['candidates'][0]['directory'])
        report = json.loads((work / 'report.json').read_text(encoding='utf-8'))['units'][0]
        measures = report.get('measures', {})
        print(f"== {unit}: code {float(measures.get('matched_code_percent', 0)):.1f}%, "
              f"data {float(measures.get('matched_data_percent', 0)):.1f}%, "
              f"functions {measures.get('matched_functions', 0)}/{measures.get('total_functions', 0)}")
        for kind in ('functions', 'sections'):
            for item in report.get(kind, []):
                percent = item.get('fuzzy_match_percent', 0) or 0
                if args.all or percent < 100:
                    print(f"   {kind[:-1]:8} {item['name']:40} {percent:6.1f}%  size={item.get('size')}")
        target = work / f'build/2049LE/obj/{unit}.o'
        built = work / manifest['profile']['object']
        if not check_paths(target, built):
            status = max(status, 2)
    return status


if __name__ == '__main__':
    sys.exit(isolated_main())
