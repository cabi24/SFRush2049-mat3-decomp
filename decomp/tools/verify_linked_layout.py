"""Fresh private link evidence for erased-helper object placement differences.

This separate route never turns a strict object-layout failure into a pass.
It freezes the ordered object inputs, invokes the existing GC/2.7 linker and
dtk into a new directory, then checks surviving function addresses/sizes/bytes,
helper stripping, and the exact trusted full-DOL exception. It accepts no saved
ELF/DOL input or old receipt as fresh evidence. Source compilation is separate.
"""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT.parent / 'tools'))
from elfio import Elf
from cwmatch import checked_symbol_bytes
from verify_layout import symbol_table
from verify_link import verify_dol

ORIGINAL_ELF_SHA256 = 'd4a06e4516ab9a75e6de780beee10ea07b8358016e4964a8baf9e8eaa282908c'


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def object_exception_scope(target, built):
    """Only permit text offsets to await linked evidence; never sizes or data."""
    wanted, target_errors = symbol_table(target)
    actual, built_errors = symbol_table(built)
    failures = [*target_errors, *built_errors]
    shifted = []
    for name, expected in wanted.items():
        found = actual.get(name, [])
        if Counter(expected) == Counter(found):
            continue
        if (len(expected) == len(found) == 1
                and expected[0][0] == found[0][0] == '.text'
                and expected[0][2:] == found[0][2:]
                and expected[0][3] == 2):
            shifted.append(name)
        else:
            failures.append(f'{name}: linked-evidence route permits only function offset changes')
    extras = sorted(set(actual) - set(wanted))
    for name in extras:
        if any(sec != '.text' or kind != 2 for sec, off, size, kind in actual[name]):
            failures.append(f'{name}: extra non-function symbol needs separate evidence')
    return failures, shifted, extras


def check_linked_functions(target_object, original, linked, extras):
    """Check each surviving target function using unambiguous trusted identities."""
    failures, evidence = [], []
    expected = [s for s in target_object.syms if s['name'] and s['type'] == 2
                and 0 < s['shndx'] < len(target_object.secs)]
    if not expected:
        return ['target object has no surviving functions to verify'], []
    for target in expected:
        name = target['name']
        originals = [s for s in original.syms if s['name'] == name and s['type'] == 2
                     and 0 < s['shndx'] < len(original.secs)]
        linked_symbols = [s for s in linked.syms if s['name'] == name and s['type'] == 2
                          and 0 < s['shndx'] < len(linked.secs)]
        if len(originals) != 1 or len(linked_symbols) != 1:
            failures.append(f'{name}: missing or ambiguous original/linked function identity')
            continue
        source, result = originals[0], linked_symbols[0]
        source_section = original.secs[source['shndx']]
        result_section = linked.secs[result['shndx']]
        if (source_section['name'] != '.text' or result_section['name'] != '.text'
                or source['size'] != target['size']
                or (source['value'], source['size']) != (result['value'], result['size'])):
            failures.append(f'{name}: linked section/address/size differs from trusted original')
            continue
        try:
            original_bytes = checked_symbol_bytes(original, source, source_section, linked=True)
            linked_bytes = checked_symbol_bytes(linked, result, result_section, linked=True)
        except ValueError as exc:
            failures.append(str(exc))
            continue
        if original_bytes != linked_bytes:
            failures.append(f'{name}: linked instruction bytes differ from trusted original')
            continue
        evidence.append({'name': name, 'address': hex(result['value']), 'size': result['size'],
                         'bytes_sha256': sha256(linked_bytes)})
    for name in extras:
        if any(s['name'] == name and s['type'] == 2
               and 0 < s['shndx'] < len(linked.secs) for s in linked.syms):
            failures.append(f'{name}: extra object helper survived the fresh link')
    return failures, evidence


def checked_input_path(path):
    # Windows sandbox may allow stat/read but deny the handle-based realpath API.
    # Use normalized absolute paths and reject reparse-point traversal explicitly.
    path = Path(os.path.abspath(path))
    for entry in (path, *path.parents):
        if getattr(entry.lstat(), 'st_file_attributes', 0) & 0x400:
            raise ValueError(f'reparse-point input path requires separate isolation evidence: {entry}')
    if not path.is_file():
        raise ValueError(f'input is not a file: {path}')
    return path


def freeze_inputs(inputs, built_object, target_object, run_dir):
    """Copy the exact ordered inputs; selected C object must replace its split."""
    inputs = [checked_input_path(path) for path in inputs]
    if len(inputs) != len(set(inputs)):
        raise ValueError('duplicate link input paths')
    if inputs.count(checked_input_path(built_object)) != 1:
        raise ValueError('selected built object must be a link input exactly once')
    if checked_input_path(target_object) in inputs:
        raise ValueError('split target object is still a link input')
    frozen_dir = run_dir / 'objects'
    frozen_dir.mkdir()
    manifest = []
    for index, source in enumerate(inputs):
        if source.suffix.lower() != '.o':
            raise ValueError(f'link input must be an object file: {source}')
        data = source.read_bytes()
        frozen = frozen_dir / f'{index:04d}.o'
        frozen.write_bytes(data)
        manifest.append({'source': str(source), 'frozen': str(frozen), 'sha256': sha256(data)})
    return manifest


def run_fresh_link(inputs, built_object, target_object, original_elf, target_dol,
                   linker_script, linker, dtk, output_root, timeout=180):
    original_data = original_elf.read_bytes()
    if sha256(original_data) != ORIGINAL_ELF_SHA256:
        raise ValueError('original ELF SHA-256 does not match the trusted recovered ELF')
    # Fail early on a wrong target before invoking any executable.
    from verify_link import TARGET_SHA256
    target_data = target_dol.read_bytes()
    if sha256(target_data) != TARGET_SHA256:
        raise ValueError('target DOL SHA-256 does not match the trusted original')
    target, built = Elf(target_object), Elf(built_object)
    failures, shifted, extras = object_exception_scope(target, built)
    if failures:
        raise ValueError('; '.join(failures))
    output_root.mkdir(parents=True, exist_ok=True)
    run_dir = Path(tempfile.mkdtemp(prefix='linked-evidence-', dir=output_root)).resolve()
    receipt = {'status': 'failed', 'run_directory': str(run_dir), 'shifted_functions': shifted,
               'extra_object_helpers': extras, 'source_compilation_verified': False}
    try:
        manifest = freeze_inputs(inputs, built_object, target_object, run_dir)
        receipt['inputs'] = manifest
        # Bind metadata checks to the exact selected frozen object consumed by the linker.
        selected = next(item for item in manifest if item['source'] == str(checked_input_path(built_object)))
        receipt['built_object_sha256'] = selected['sha256']
        if selected['sha256'] != sha256(built.d):
            raise ValueError('selected object changed while freezing inputs')
        script = run_dir / 'ldscript.lcf'
        script.write_bytes(linker_script.read_bytes())
        original_snapshot = run_dir / 'trusted-original.elf'
        original_snapshot.write_bytes(original_data)
        receipt['linker_script_sha256'] = sha256(script.read_bytes())
        receipt['linker_sha256'] = sha256(linker.read_bytes())
        receipt['dtk_sha256'] = sha256(dtk.read_bytes())
        receipt['target_object_sha256'] = sha256(target.d)
        receipt['original_elf_sha256'] = sha256(original_data)
        receipt['target_dol_sha256'] = sha256(target_data)
        response = run_dir / 'objects.rsp'
        response.write_text('\n'.join(f'"{item["frozen"]}"' for item in manifest) + '\n', encoding='utf-8')
        linked_elf, linked_dol = run_dir / 'fresh.elf', run_dir / 'fresh.dol'
        commands = [[str(linker.resolve()), '-fp', 'hardware', '-nodefaults', '-lcf', str(script),
                     '-o', str(linked_elf), '@' + str(response)],
                    [str(dtk.resolve()), 'elf2dol', str(linked_elf), str(linked_dol)]]
        receipt['commands'] = commands
        temporary = run_dir / 'temporary'
        temporary.mkdir()
        environment = {**os.environ, 'TEMP': str(temporary), 'TMP': str(temporary)}
        for index, command in enumerate(commands):
            # These paths are new; the route has no option to consume an old link artifact.
            output = linked_elf if index == 0 else linked_dol
            if output.exists():
                raise ValueError(f'fresh output already exists before command: {output}')
            result = subprocess.run(command, cwd=run_dir, capture_output=True, text=True,
                                    stdin=subprocess.DEVNULL, timeout=timeout, check=False, env=environment)
            (run_dir / f'command-{index}.txt').write_text(result.stdout + result.stderr, encoding='utf-8')
            if result.returncode or not output.is_file() or not output.stat().st_size:
                raise ValueError(f'fresh command {index} failed: exit {result.returncode}')
        if any(sha256(Path(item['frozen']).read_bytes()) != item['sha256'] for item in manifest):
            raise ValueError('a frozen link input changed during execution')
        failures, functions = check_linked_functions(target, Elf(original_snapshot), Elf(linked_elf), extras)
        failures.extend(verify_dol(linked_dol.read_bytes(), target_data))
        receipt.update(functions=functions, failures=failures,
                       linked_elf_sha256=sha256(linked_elf.read_bytes()),
                       linked_dol_sha256=sha256(linked_dol.read_bytes()))
        if not failures:
            receipt['status'] = 'passed'
    except (OSError, ValueError, subprocess.TimeoutExpired) as exc:
        receipt['failures'] = [str(exc)]
    (run_dir / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    return receipt


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('unit', help='e.g. NGC/ngcMemcard; build-root contains obj/ and src/')
    parser.add_argument('--inputs-json', type=Path, required=True, help='ordered JSON array of .o paths')
    parser.add_argument('--project-root', type=Path, default=ROOT, help='base for relative manifest paths')
    parser.add_argument('--build-root', type=Path, default=ROOT / 'build/2049LE')
    parser.add_argument('--original-elf', type=Path, default=ROOT.parent / 'rush_d.elf')
    parser.add_argument('--target-dol', type=Path, default=ROOT / 'orig/2049LE/files/rush_d.dol')
    parser.add_argument('--linker', type=Path, default=ROOT / 'build/compilers/GC/2.7/mwldeppc.exe')
    parser.add_argument('--dtk', type=Path, default=ROOT / 'build/tools/dtk.exe')
    parser.add_argument('--output-root', type=Path, required=True, help='private writable evidence directory')
    args = parser.parse_args(argv)
    try:
        manifest = json.loads(args.inputs_json.read_text(encoding='utf-8'))
        if not isinstance(manifest, list) or not all(isinstance(item, str) for item in manifest):
            raise ValueError('inputs JSON must be an ordered array of object path strings')
        root = args.build_root.resolve()
        unit = Path(args.unit + '.o')
        if unit.is_absolute() or '..' in unit.parts:
            raise ValueError('unit must be a relative unit path')
        receipt = run_fresh_link([args.project_root / item for item in manifest], root / 'src' / unit,
                                 root / 'obj' / unit, args.original_elf, args.target_dol,
                                 root / 'ldscript.lcf', args.linker, args.dtk, args.output_root)
    except (OSError, ValueError) as exc:
        print(f'FAIL: {exc}')
        return 2
    print(f'Fresh linked evidence {receipt["status"]}: {receipt["run_directory"]}/receipt.json')
    for failure in receipt.get('failures', []):
        print(f'  {failure}')
    return 0 if receipt['status'] == 'passed' else 2


if __name__ == '__main__':
    sys.exit(main())
