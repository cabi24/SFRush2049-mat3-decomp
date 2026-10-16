"""Relink and verify the exact recorded GC/2.7 DOL exception.

The trusted target is pinned by SHA-256. The exception is fixed evidence, never
learned from the build under test. See docs/verification-baseline.md.
Use --no-build to verify existing artifacts without running configure or Ninja.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
TARGET_SHA256 = '27eac444d9fee753864ef2978cc8c1999f07eb8d459d0dee60f254c18cd05e67'
# (segment, virtual address, target byte, built byte). Four .dtors pointer
# bytes plus the .dtors size in _rom_copy_info; not five arbitrary bytes.
KNOWN_DIFFERENCES = frozenset({
    ('T0', 0x80005573, 0x08, 0x04),
    ('D1', 0x801086A0, 0x80, 0x00),
    ('D1', 0x801086A1, 0x0E, 0x00),
    ('D1', 0x801086A2, 0x19, 0x00),
    ('D1', 0x801086A3, 0xB8, 0x00),
})


def read_dol(data):
    """Parse loaded segments, rejecting truncated or overlapping extents."""
    if len(data) < 0x100:
        raise ValueError('DOL header is truncated')
    offsets = struct.unpack_from('>18I', data, 0)
    addresses = struct.unpack_from('>18I', data, 0x48)
    sizes = struct.unpack_from('>18I', data, 0x90)
    segments, file_ranges, memory_ranges = {}, [], []
    for i, (off, addr, size) in enumerate(zip(offsets, addresses, sizes)):
        if not size:
            continue
        key = f'T{i}' if i < 7 else f'D{i - 7}'
        if off < 0x100 or off + size > len(data) or addr + size > 0x100000000:
            raise ValueError(f'{key}: invalid file or memory extent')
        for start, end in file_ranges:
            if off < end and start < off + size:
                raise ValueError(f'{key}: overlapping file extent')
        for start, end in memory_ranges:
            if addr < end and start < addr + size:
                raise ValueError(f'{key}: overlapping memory extent')
        file_ranges.append((off, off + size))
        memory_ranges.append((addr, addr + size))
        segments[key] = (addr, data[off:off + size])
    bss = struct.unpack_from('>II', data, 0xD8)
    if bss[0] + bss[1] > 0x100000000:
        raise ValueError('BSS extent overflows address space')
    return segments, bss, struct.unpack_from('>I', data, 0xE0)[0]


def verify_dol(built_data, target_data):
    """Return failures. Equality here covers loaded bytes and loader metadata."""
    if hashlib.sha256(target_data).hexdigest() != TARGET_SHA256:
        return ['target SHA-256 does not match the trusted original']
    try:
        built, bbss, bent = read_dol(built_data)
        target, tbss, tent = read_dol(target_data)
    except ValueError as exc:
        return [str(exc)]
    failures, differences = [], set()
    if set(built) != set(target):
        failures.append('loaded segment set differs from target')
    if bbss != tbss:
        failures.append(f'BSS differs: built {bbss}, target {tbss}')
    if bent != tent:
        failures.append(f'entry differs: built {bent:#x}, target {tent:#x}')
    for key in sorted(set(built) & set(target)):
        ba, bd = built[key]
        ta, td = target[key]
        if ba != ta or len(bd) != len(td):
            failures.append(f'{key}: address/size differs: built {ba:#x}+{len(bd):#x}, '
                            f'target {ta:#x}+{len(td):#x}')
        differences.update((key, ta + i, y, x)
                           for i, (x, y) in enumerate(zip(bd, td)) if x != y)
    for key, addr, target_byte, built_byte in sorted(differences - KNOWN_DIFFERENCES)[:25]:
        failures.append(f'unexpected difference {key} {addr:#010x}: '
                        f'target {target_byte:02x}, built {built_byte:02x}')
    for key, addr, target_byte, built_byte in sorted(KNOWN_DIFFERENCES - differences):
        failures.append(f'missing known exception {key} {addr:#010x}: '
                        f'target {target_byte:02x}, built {built_byte:02x}')
    return failures


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--no-build', action='store_true', help='verify existing artifacts only')
    ap.add_argument('--built', type=Path, default=ROOT / 'build/2049LE/rush_d.dol')
    ap.add_argument('--target', type=Path, default=ROOT / 'orig/2049LE/files/rush_d.dol')
    args = ap.parse_args(argv)
    if not args.no_build:
        from check import CONFIGURE_ARGS, NINJA
        subprocess.run([sys.executable, 'configure.py', *CONFIGURE_ARGS], cwd=ROOT,
                       capture_output=True, stdin=subprocess.DEVNULL, timeout=300, check=True)
        result = subprocess.run([NINJA, 'build/2049LE/rush_d.dol'], cwd=ROOT,
                                capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=900)
        if result.returncode:
            lines = [line for line in (result.stdout + result.stderr).splitlines()
                     if 'Alignment for' not in line and 'Missing configuration' not in line]
            print('\n'.join(lines[-25:]))
            return 1
    try:
        failures = verify_dol(args.built.read_bytes(), args.target.read_bytes())
    except OSError as exc:
        failures = [str(exc)]
    if failures:
        print('FAIL: DOL does not reproduce the trusted GC/2.7 exception')
        print('\n'.join(f'  {failure}' for failure in failures))
        return 2
    print('PASS: loaded segments, BSS and entry match the trusted target with exactly '
          'the five recorded GC/2.7 destructor exception bytes')
    return 0


if __name__ == '__main__':
    sys.exit(main())
