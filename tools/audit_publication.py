"""Audit the explicit source-only allowlist and optionally the exact Git index."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--git', help='Git executable; also compare the complete index with the allowlist')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    allowlist = json.loads((root / 'docs/file-allowlist.json').read_text())
    entries = allowlist['files']
    paths = [e['path'] for e in entries]
    failures = []
    if len(paths) != len(set(paths)):
        failures.append('Duplicate allowlist path')
    denied_dirs = {'orig', 'build', 'assets', 'dl', 'work', '.aws', '.ssh', '.git', 'generated'}
    allowed_ext = {'.py', '.c', '.cpp', '.h', '.json', '.md', '.txt', '.patch', '.yml', '.tpl', '.sha1'}
    special = {'.gitignore', '.gitattributes'}
    secret_patterns = [
        re.compile(r'gh[pousr]_[A-Za-z0-9]{20,}'),
        re.compile(r'github_pat_[A-Za-z0-9_]{30,}'),
        re.compile(r'AKIA[0-9A-Z]{16}'),
        re.compile(r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----'),
        re.compile(r'(?:sk-proj-|sk-)[A-Za-z0-9_-]{30,}'),
        re.compile(r'https?://[^\s/:]+:[^\s/@]+@'),
    ]
    private_path = re.compile(r'[A-Za-z]:[\\/]+Users[\\/]|/' + r'home/[^\s/]+|/' + r'Users/[^\s/]+', re.I)
    large_encoded = re.compile(r'[A-Za-z0-9+/]{500,}={0,2}|(?:\\x[0-9a-fA-F]{2}){64,}')
    raw_assembly = re.compile(r'^/\* [0-9A-F]{8} [0-9A-F]{8}  [0-9A-F ]{11} \*/', re.M)
    binary_magic = (b'\x7fELF', b'MZ', b'PK\x03\x04', b'\x89PNG', b'\x1f\x8b')
    git_entries = {}
    if args.git:
        result = subprocess.run([args.git, 'ls-files', '--stage', '-z'], cwd=root,
                                check=True, capture_output=True)
        for record in result.stdout.split(b'\0'):
            if not record:
                continue
            metadata, path = record.split(b'\t', 1)
            mode, blob, stage = metadata.decode().split()
            git_entries[path.decode()] = (mode, blob, stage)
        if set(git_entries) != set(paths):
            failures.append('Git index path set differs from allowlist')
    total_bytes = 0
    for entry in entries:
        rel = Path(entry['path'])
        target = root / rel
        if rel.is_absolute() or '..' in rel.parts or any(p in denied_dirs for p in rel.parts):
            failures.append('Excluded or unsafe path: ' + str(rel))
            continue
        if target.is_symlink() or not target.is_file():
            failures.append('Missing file or symbolic link: ' + str(rel))
            continue
        if rel.name not in special and rel.suffix not in allowed_ext:
            failures.append('Unapproved extension: ' + str(rel))
        data = target.read_bytes()
        total_bytes += len(data)
        if b'\0' in data or data.startswith(binary_magic):
            failures.append('Binary signature: ' + str(rel))
        try:
            text = data.decode('utf-8')
        except UnicodeDecodeError:
            failures.append('Non-UTF-8 file: ' + str(rel))
            continue
        if entry.get('sha256') and (digest(data) != entry['sha256'] or len(data) != entry['bytes']):
            failures.append('Allowlist hash/size mismatch: ' + str(rel))
        if any(p.search(text) for p in secret_patterns):
            failures.append('Secret indicator: ' + str(rel))
        if private_path.search(text):
            failures.append('Private machine path: ' + str(rel))
        if large_encoded.search(text) or raw_assembly.search(text):
            failures.append('Encoded payload or raw disassembly indicator: ' + str(rel))
        if args.git and entry['path'] in git_entries:
            mode, blob, stage = git_entries[entry['path']]
            actual = hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()
            if blob != actual or stage != '0' or mode != '100644':
                failures.append('Git blob, mode or merge-stage mismatch: ' + str(rel))
    source = json.loads((root/'docs/provenance/source-baseline.json').read_text())
    for item in source['files']:
        if digest((root/item['path']).read_bytes()) != item['export_sha256']:
            failures.append('Source baseline mismatch: ' + item['path'])
    progress = json.loads((root/'progress.json').read_text())
    for field in ['matched_functions', 'total_functions', 'matched_code', 'total_code']:
        if sum(int(u['measures'].get(field, 0)) for u in progress['units']) != progress[field]:
            failures.append('Progress sum mismatch: ' + field)
    config = (root/'decomp/configure.py').read_text()
    if len(re.findall(r'Object\(Matching, "([^"]+)"', config)) != progress['configured_C_linked_units']:
        failures.append('Configured linked-unit count mismatch')
    result = {'status':'passed' if not failures else 'failed', 'files':len(paths),
              'bytes':total_bytes, 'exact_Git_index_checked':bool(args.git),
              'selected_reviewed_file_hashes_checked':len(source['files']),
              'failures':failures,
              'scope':'Pattern-based secret/binary scan, explicit allowlist, source hashes and progress consistency. Does not prove absence of every possible secret or grant redistribution rights.'}
    print(json.dumps(result, indent=2))
    raise SystemExit(bool(failures))


if __name__ == '__main__':
    main()
