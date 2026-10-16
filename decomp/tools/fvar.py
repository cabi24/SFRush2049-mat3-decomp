"""Try bounded text variants of a real TU without rewriting canonical source.

python tools/fvar.py <unit> <func> <old> <new> [<new> ...]
    --output-root <private-dir> --max-variants <bound> --timeout <seconds>
The bound includes the original candidate. Literal backslash-n means newline.
"""
import argparse
import json
from pathlib import Path
import sys
import tempfile

import candidate


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('unit')
    parser.add_argument('function')
    parser.add_argument('old')
    parser.add_argument('new', nargs='+')
    parser.add_argument('--project', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--tool-root', type=Path)
    parser.add_argument('--output-root', type=Path, required=True)
    parser.add_argument('--max-variants', type=int, required=True)
    parser.add_argument('--timeout', type=int, required=True)
    args = parser.parse_args(argv)
    if not 1 <= args.max_variants <= candidate.MAX_VARIANTS or not 1 <= args.timeout <= 300:
        parser.error('Bounds: --max-variants 1..16, --timeout 1..300')
    if len(args.new) + 1 > args.max_variants:
        parser.error('Candidate bound includes the original plus all replacements')
    project, output = args.project.resolve(), args.output_root.resolve()
    if candidate.inside(output, project) or candidate.inside(project, output):
        parser.error('Output root must be disjoint from input project')
    try:
        config = json.loads((project / 'objdiff.json').read_text(encoding='utf-8'))
        units = [unit for unit in config['units'] if unit['name'].removeprefix('rush_d/') == args.unit]
        if len(units) != 1:
            raise ValueError(f'Unit has no unique source: {args.unit}')
        source = project / units[0]['metadata']['source_path']
        output.mkdir(parents=True, exist_ok=True)
        private = Path(tempfile.mkdtemp(prefix='mat3-fvar-', dir=output))
        baseline = private / ('original' + source.suffix)
        records = []
        candidate.copy_input(source, baseline, records)
        text = baseline.read_text(encoding='utf-8')
        old = args.old.replace('\\n', '\n')
        if not old or text.count(old) != 1:
            raise ValueError('Old text must occur exactly once in the complete TU')
        candidates = [baseline]
        for index, replacement in enumerate(args.new):
            path = private / (f'variant-{index:02d}' + source.suffix)
            path.write_bytes(text.replace(old, replacement.replace('\\n', '\n'), 1).encode('utf-8'))
            candidates.append(path)
        candidate.write_json(private / 'variant-inputs.json', records)
        options = argparse.Namespace(unit=args.unit, function=args.function, project=project,
            output_root=output, tool_root=args.tool_root, max_variants=args.max_variants,
            timeout=args.timeout, candidate=candidates)
        code, _ = candidate.run(options)
        return code
    except (OSError, ValueError, KeyError) as error:
        print(f'fvar: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
