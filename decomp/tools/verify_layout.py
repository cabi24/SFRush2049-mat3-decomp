"""Strict object symbol placement and extent checks, independent of the runner.

Extra symbols are reported but cannot establish final linked equality. Erased
helpers and unused inline statics require a separate verified full link.
"""
from collections import Counter, defaultdict
import re


def symbol_table(elf):
    table = defaultdict(list)
    failures = []
    for symbol in elf.syms:
        name = symbol['name']
        if not name or symbol['type'] in (3, 4) or not 0 < symbol['shndx'] < len(elf.secs):
            continue
        if name.startswith(('gap_', '@', '...', 'lbl_')):
            continue
        section = elf.secs[symbol['shndx']]
        offset, size = symbol['value'], symbol['size']
        if offset < 0 or size < 0 or offset + size > section['size']:
            failures.append(f'invalid extent {name}: {section["name"]}+{offset:#x} '
                            f'size {size:#x}, section size {section["size"]:#x}')
        # Keep all entries: normalizing local-static counters must not overwrite
        # a colliding symbol and silently lose a placement/extent requirement.
        key = re.sub(r'\$\d+$', '$', name)
        table[key].append((section['name'], offset, size, symbol['type']))
    return table, failures


def compare_layout(target, built):
    target_table, target_errors = symbol_table(target)
    built_table, built_errors = symbol_table(built)
    failures = [f'target: {error}' for error in target_errors]
    failures.extend(f'built: {error}' for error in built_errors)
    total = sum(len(entries) for entries in target_table.values())
    matched = 0
    for name, expected in sorted(target_table.items()):
        wanted, actual = Counter(expected), Counter(built_table.get(name, []))
        matched += sum((wanted & actual).values())
        if wanted != actual:
            failures.append(f'layout mismatch {name}: built {sorted(actual.elements())}, '
                            f'target {sorted(wanted.elements())} (section, offset, size, type)')
    extra = sorted(set(built_table) - set(target_table))
    return total, matched, failures, extra


def check_paths(target_path, built_path):
    # elfio belongs to the parent project's tools directory.
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools'))
    from elfio import Elf
    try:
        total, matched, failures, extra = compare_layout(Elf(target_path), Elf(built_path))
    except (OSError, ValueError, AssertionError, IndexError) as exc:
        print(f'   layout: FAIL: {exc}')
        return False
    print(f'   layout: {matched}/{total} target symbols match section/offset/size/type')
    for failure in failures[:25]:
        print(f'      {failure}')
    if extra:
        print(f'      extra symbols in built object (not validated by target layout): {extra[:8]}')
        print('      a verified full link is required to validate stripping and final addresses')
    return not failures
