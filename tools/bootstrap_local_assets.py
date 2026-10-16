"""Generate ignored art initializer includes from a user-supplied original DOL."""
import argparse
import hashlib
import json
from pathlib import Path
import struct


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dol', type=Path, default=root / 'decomp/orig/2049LE/files/rush_d.dol')
    args = parser.parse_args()
    recipe = json.loads((root / 'docs/provenance/local-assets.json').read_text())
    data = args.dol.read_bytes()
    if hashlib.sha256(data).hexdigest() != recipe['original_dol_sha256']:
        raise SystemExit('Original DOL hash mismatch; no files generated.')
    offsets = struct.unpack_from('>18I', data, 0)
    addresses = struct.unpack_from('>18I', data, 0x48)
    sizes = struct.unpack_from('>18I', data, 0x90)
    pending = []
    for asset in recipe['assets']:
        address = int(asset['address'], 16)
        candidates = [o + address - a for o, a, s in zip(offsets, addresses, sizes)
                      if s and a <= address and address + asset['bytes'] <= a + s
                      and o + s <= len(data)]
        if len(candidates) != 1:
            raise SystemExit('Ambiguous or invalid original segment for ' + asset['name'])
        off = candidates[0]
        payload = data[off:off + asset['bytes']]
        if hashlib.sha256(payload).hexdigest() != asset['sha256']:
            raise SystemExit('Asset hash mismatch: ' + asset['name'])
        values = [str(v - 256 if asset['signed'] and v >= 128 else v) for v in payload]
        content = ''.join('    ' + ', '.join(values[i:i+24]) + ',\n'
                          for i in range(0, len(values), 24)).encode('ascii')
        target = root / asset['generated_include']
        if not target.resolve().is_relative_to(root / 'decomp/include/generated'):
            raise SystemExit('Invalid generation destination')
        if target.exists() and target.read_bytes() != content:
            raise SystemExit('Refusing to overwrite a different local file: ' + str(target))
        pending.append((target, content))
    for target, content in pending:
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(content)
        print(target.relative_to(root))


if __name__ == '__main__':
    main()
