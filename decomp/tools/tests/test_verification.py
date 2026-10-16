"""Verification controls and adversarial mutations; no compiler/network needed.

Run from decomp: python -B -m unittest discover -s tools/tests -v
Real-asset tests use local originals and accepted build objects, never embedded
or downloaded fixtures. Missing local assets are explicitly skipped.
"""
import contextlib
import io
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
TEST_OUTPUT = ROOT / 'build/verification-tests'
TEST_OUTPUT.mkdir(parents=True, exist_ok=True)
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT.parent / 'tools'))
import check
import cwmatch
from elfio import Elf
import verify_layout
import verify_link
import verify_linked_layout


class DolVerificationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        paths = [ROOT / 'orig/2049LE/files/rush_d.dol',
                 ROOT / 'build/2049LE/link_2.7.dol', ROOT / 'build/2049LE/rush_d.dol']
        if not all(path.exists() for path in paths):
            raise unittest.SkipTest('requires the local target and two recorded accepted DOLs')
        cls.target, cls.split, cls.accepted = [path.read_bytes() for path in paths]

    def mutate_byte(self, address, value):
        result = bytearray(self.accepted)
        offsets = struct.unpack_from('>18I', result, 0)
        addresses = struct.unpack_from('>18I', result, 0x48)
        sizes = struct.unpack_from('>18I', result, 0x90)
        for off, addr, size in zip(offsets, addresses, sizes):
            if addr <= address < addr + size:
                result[off + address - addr] = value
                return result
        self.fail(f'address {address:#x} not in a segment')

    def test_recorded_split_and_accepted_controls(self):
        self.assertEqual(self.split, self.accepted)
        for built in (self.split, self.accepted):
            self.assertEqual(verify_link.verify_dol(built, self.target), [])

    def test_changed_exception_value_rejected_at_same_diff_count(self):
        built = self.mutate_byte(0x801086A0, 1)
        self.assertTrue(verify_link.verify_dol(built, self.target))

    def test_moved_exception_rejected_at_same_diff_count(self):
        built = self.mutate_byte(0x801086A0, 0x80)
        # Move that one differing byte from .dtors into the next unchanged byte.
        offsets = struct.unpack_from('>18I', built, 0)
        built[offsets[8] + 4] = 1
        self.assertTrue(verify_link.verify_dol(built, self.target))

    def test_new_difference_rejected(self):
        self.assertTrue(verify_link.verify_dol(self.mutate_byte(0x801086A4, 1), self.target))

    def test_exact_original_is_not_the_recorded_linker_signature(self):
        self.assertTrue(verify_link.verify_dol(self.target, self.target))

    def test_changed_target_cannot_bless_build(self):
        target = bytearray(self.target)
        target[-1] ^= 1
        self.assertIn('target SHA-256', verify_link.verify_dol(self.accepted, target)[0])

    def test_loader_metadata_mutations_rejected(self):
        for offset in (0x48, 0x90, 0xD8, 0xDC, 0xE0):
            with self.subTest(header_offset=hex(offset)):
                built = bytearray(self.accepted)
                value = struct.unpack_from('>I', built, offset)[0]
                struct.pack_into('>I', built, offset, value + 4)
                self.assertTrue(verify_link.verify_dol(built, self.target))

    def test_missing_segment_rejected(self):
        built = bytearray(self.accepted)
        struct.pack_into('>I', built, 0x90 + 7 * 4, 0)
        self.assertTrue(verify_link.verify_dol(built, self.target))

    def test_truncated_segment_rejected(self):
        self.assertTrue(verify_link.verify_dol(self.accepted[:-32], self.target))

    def test_overlapping_extent_rejected(self):
        built = bytearray(self.accepted)
        struct.pack_into('>I', built, 4, 0x100)
        self.assertTrue(verify_link.verify_dol(built, self.target))

    def test_cli_returns_failure_for_mutated_dol(self):
        with tempfile.TemporaryDirectory(prefix='verify_dol_', dir=TEST_OUTPUT) as directory:
            built = Path(directory) / 'bad.dol'
            built.write_bytes(self.mutate_byte(0x801086A0, 1))
            result = subprocess.run([sys.executable, '-B', str(ROOT / 'tools/verify_link.py'),
                                     '--no-build', '--built', str(built)],
                                    capture_output=True, text=True, check=False)
            self.assertEqual(result.returncode, 2, result.stdout + result.stderr)


def elf_fixture(symbols):
    return SimpleNamespace(
        secs=[{'name': '', 'size': 0}, {'name': '.text', 'size': 64},
              {'name': '.data', 'size': 64}], syms=symbols)


def symbol(name='foo', offset=0, size=16, kind=2, section=1):
    return {'name': name, 'value': offset, 'size': size, 'type': kind, 'shndx': section}


class LayoutTests(unittest.TestCase):
    def test_known_good_and_local_counter_control(self):
        target = elf_fixture([symbol(), symbol('local$12', 16, 4, 1, 2)])
        built = elf_fixture([symbol(), symbol('local$99', 16, 4, 1, 2)])
        self.assertEqual(verify_layout.compare_layout(target, built), (2, 2, [], []))

    def test_position_size_section_type_and_extent_mutations(self):
        target = elf_fixture([symbol()])
        for changes in ({'offset': 4}, {'size': 20}, {'section': 2},
                        {'kind': 1}, {'offset': 60}, {'size': 0}):
            with self.subTest(changes=changes):
                self.assertTrue(verify_layout.compare_layout(target, elf_fixture([symbol(**changes)]))[2])

    def test_missing_symbol_rejected(self):
        self.assertTrue(verify_layout.compare_layout(elf_fixture([symbol()]), elf_fixture([]))[2])

    def test_colliding_normalized_names_do_not_overwrite(self):
        target = elf_fixture([symbol('a$1'), symbol('a$2', 16)])
        bad = elf_fixture([symbol('a$3', 16)])
        self.assertTrue(verify_layout.compare_layout(target, bad)[2])
        good = elf_fixture([symbol('a$9'), symbol('a$8', 16)])
        self.assertEqual(verify_layout.compare_layout(target, good)[2], [])

    def test_missing_objects_fail(self):
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertFalse(verify_layout.check_paths('does-not-exist.o', 'also-missing.o'))

    def test_runner_returns_layout_failure(self):
        report = {'units': [{'name': 'rush_d/test', 'measures': {}}]}
        with tempfile.TemporaryDirectory(prefix='verify_runner_', dir=TEST_OUTPUT) as directory:
            root = Path(directory)
            (root / 'build/2049LE').mkdir(parents=True)
            (root / 'build/2049LE/report.json').write_text(json.dumps(report))
            with patch.object(check, 'ROOT', str(root)), \
                    patch.object(check.subprocess, 'run', return_value=SimpleNamespace(returncode=0, stdout='', stderr='')), \
                    patch.object(sys, 'argv', ['check.py', 'test']), \
                    contextlib.redirect_stdout(io.StringIO()):
                # Missing real objects must propagate to the runner's exit status.
                self.assertEqual(check.main(), 2)
                with patch.object(check, 'layout_check', return_value=True):
                    self.assertEqual(check.main(), 0)


class RelocationTests(unittest.TestCase):
    def compare(self, built_rel=None, original_rel=None, built_bytes=None):
        code = bytes.fromhex('480000014e800020')  # bl; blr
        built = {'f': (0, built_bytes if built_bytes is not None else code)}
        original = ({'f': [(0x80006000, code)]},
                    {0x80006000 + off: value for off, value in (original_rel or {}).items()})
        with patch.object(cwmatch, '_orig', original):
            return cwmatch.compare('f', built, built_rel or {})

    def test_raw_and_relocation_controls(self):
        self.assertEqual(self.compare(), 'MATCH')
        rel = {0: (10, 'callee', 0)}
        self.assertEqual(self.compare(rel, rel), 'MATCH')

    def test_relocation_type_target_and_addend_mutations_rejected(self):
        original = {0: (10, 'callee', 0)}
        for changed in ((11, 'callee', 0), (10, 'wrong_callee', 0), (10, 'callee', 4)):
            with self.subTest(relocation=changed):
                self.assertIn('relocation differences', self.compare({0: changed}, original))

    def test_missing_or_unnamed_relocation_is_candidate(self):
        rel = {0: (10, 'callee', 0)}
        self.assertTrue(self.compare(rel, {}).startswith('CANDIDATE'))
        self.assertTrue(self.compare({}, rel).startswith('CANDIDATE'))
        self.assertTrue(self.compare({0: (10, '', 0)}, {0: (10, '', 0)}).startswith('CANDIDATE'))

    def test_unsupported_relocation_is_candidate(self):
        rel = {0: (1, 'callee', 0)}
        self.assertTrue(self.compare(rel, rel).startswith('CANDIDATE'))

    def test_relocation_offset_mutation_not_match(self):
        self.assertNotEqual(self.compare({4: (10, 'callee', 0)}, {0: (10, 'callee', 0)}), 'MATCH')

    def test_invalid_relocation_extent_not_match(self):
        for off in (1, 2, 7):
            with self.subTest(offset=off):
                rel = {off: (10, 'callee', 0)}
                self.assertIn('invalid relocation extent', self.compare(rel, rel))

    def test_instruction_mutation_not_match(self):
        self.assertNotEqual(self.compare(built_bytes=bytes.fromhex('480000014e800021')), 'MATCH')

    def test_function_extent_mutation_not_match(self):
        self.assertIn('size', self.compare(built_bytes=bytes.fromhex('48000001')))

    def test_ambiguous_original_does_not_match(self):
        code = bytes.fromhex('4e800020')
        with patch.object(cwmatch, '_orig', ({'f': [(0x100, code), (0x200, code)]}, {})):
            self.assertIn('ambiguous-orig', cwmatch.compare('f', {'f': (0, code)}, {}))

    def test_counter_names_require_evidence_never_blind_equivalence(self):
        for built, original in [('sInited$74', 'sInited$49'), ('@112', '@71')]:
            with self.subTest(names=(built, original)):
                result = self.compare({0: (10, built, 0)}, {0: (10, original, 0)})
                self.assertTrue(result.startswith('CANDIDATE'), result)
                self.assertIn('needs evidence', result)
        self.assertIn('relocation differences',
                      self.compare({0: (10, 'wrong$74', 0)}, {0: (10, 'sInited$49', 0)}))
        self.assertIn('relocation differences',
                      self.compare({0: (10, '@112', 4)}, {0: (10, '@71', 0)}))


class OriginalExtractionTests(unittest.TestCase):
    def original(self, value=0x80006000, size=8, section_size=16, file_size=16, section_offset=0,
                 relocations=()):
        text = {'idx': 1, 'name': '.text', 'addr': 0x80006000, 'off': section_offset, 'size': section_size}
        return SimpleNamespace(d=bytes(file_size), by_name={'.text': text},
                               syms=[dict(symbol('f', value, size), index=0)],
                               relocs=lambda section: iter(relocations))

    def test_original_extent_control(self):
        elf = self.original()
        with patch.object(cwmatch, 'Elf', return_value=elf), patch.object(cwmatch, '_orig', None):
            funcs, relocations = cwmatch.orig_funcs()
            self.assertEqual(funcs['f'], [(0x80006000, bytes(8))])
            self.assertEqual(relocations, {})

    def test_original_bounds_rejected_before_slice(self):
        for fields in [{'value': 0x80005FFC}, {'value': 0x80006010}, {'size': 20},
                       {'section_size': 20}, {'file_size': 4}, {'section_offset': 12}, {'size': 7}]:
            with self.subTest(fields=fields):
                with patch.object(cwmatch, 'Elf', return_value=self.original(**fields)), \
                        patch.object(cwmatch, '_orig', None):
                    with self.assertRaisesRegex(ValueError, 'invalid .*extent'):
                        cwmatch.orig_funcs()
                    self.assertIsNone(cwmatch._orig)

    def test_duplicate_original_relocations_rejected(self):
        for second_addend in (0, 4):
            rows = [(0x80006000, 10, 0, 0), (0x80006000, 10, 0, second_addend)]
            with self.subTest(addend=second_addend), \
                    patch.object(cwmatch, 'Elf', return_value=self.original(relocations=rows)), \
                    patch.object(cwmatch, '_orig', None):
                with self.assertRaisesRegex(ValueError, 'duplicate original relocation'):
                    cwmatch.orig_funcs()
                self.assertIsNone(cwmatch._orig)

    def test_original_relocation_outside_text_rejected(self):
        for offset in (0x80005FFC, 0x80006010):
            rows = [(offset, 10, 0, 0)]
            with self.subTest(offset=offset), \
                    patch.object(cwmatch, 'Elf', return_value=self.original(relocations=rows)), \
                    patch.object(cwmatch, '_orig', None):
                with self.assertRaisesRegex(ValueError, 'original relocation outside'):
                    cwmatch.orig_funcs()


class FreshLinkedEvidenceTests(unittest.TestCase):
    def linked_fixture(self, address=0x80006000, size=8, code=bytes(8), helper=False):
        text = {'idx': 1, 'name': '.text', 'addr': 0x80006000, 'off': 0, 'size': 32}
        syms = [symbol('f', address, size)]
        if helper:
            syms.append(symbol('erased', 0x80006008, 4))
        return SimpleNamespace(d=code + bytes(32 - len(code)), secs=[{'name': ''}, text], syms=syms)

    def test_only_offsets_can_await_linked_evidence(self):
        target = elf_fixture([symbol(), symbol('data', 0, 4, 1, 2)])
        good = elf_fixture([symbol(offset=4), symbol('data', 0, 4, 1, 2), symbol('erased', 32, 4)])
        self.assertEqual(verify_linked_layout.object_exception_scope(target, good), ([], ['foo'], ['erased']))
        for changed in [elf_fixture([symbol(size=20), symbol('data', 0, 4, 1, 2)]),
                        elf_fixture([symbol(), symbol('data', 4, 4, 1, 2)])]:
            self.assertTrue(verify_linked_layout.object_exception_scope(target, changed)[0])

    def test_linked_address_size_bytes_and_helper_stripping(self):
        target = elf_fixture([symbol('f', 0, 8)])
        original = self.linked_fixture()
        failures, functions = verify_linked_layout.check_linked_functions(target, original, self.linked_fixture(), ['erased'])
        self.assertEqual(failures, [])
        self.assertEqual(len(functions), 1)
        for fields in [{'address': 0x80006004}, {'size': 12}, {'code': bytes.fromhex('48000001')}, {'helper': True}]:
            with self.subTest(fields=fields):
                self.assertTrue(verify_linked_layout.check_linked_functions(
                    target, original, self.linked_fixture(**fields), ['erased'])[0])

    def test_inputs_must_include_selected_build_and_exclude_split(self):
        with tempfile.TemporaryDirectory(prefix='fresh_inputs_', dir=TEST_OUTPUT) as directory:
            root = Path(directory)
            built, target = root / 'built.o', root / 'target.o'
            built.write_bytes(b'built')
            target.write_bytes(b'target')
            run = root / 'control'
            run.mkdir()
            manifest = verify_linked_layout.freeze_inputs([built], built, target, run)
            self.assertEqual(Path(manifest[0]['frozen']).read_bytes(), b'built')
            built.write_bytes(b'changed after freezing')
            self.assertEqual(Path(manifest[0]['frozen']).read_bytes(), b'built')
            for index, inputs in enumerate(([target], [built, target], [built, built])):
                run = root / f'bad-{index}'
                run.mkdir()
                with self.subTest(inputs=inputs), self.assertRaises(ValueError):
                    verify_linked_layout.freeze_inputs(inputs, built, target, run)

    def test_no_saved_artifact_cli_option(self):
        # Fresh evidence cannot be requested by supplying an existing saved ELF/DOL.
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as result:
            verify_linked_layout.main(['NGC/ngcMemcard', '--inputs-json', 'inputs.json',
                                       '--output-root', 'private-output', '--linked-elf', 'historical.elf'])
        self.assertEqual(result.exception.code, 2)

    def test_reparse_point_input_requires_isolation_evidence(self):
        with patch.object(Path, 'lstat', return_value=SimpleNamespace(st_file_attributes=0x400)):
            with self.assertRaisesRegex(ValueError, 'reparse-point input'):
                verify_linked_layout.checked_input_path('input.o')

    def test_tool_success_without_new_output_cannot_use_historical_link(self):
        built = ROOT / 'build/2049LE/src/NGC/ngcMemcard.o'
        target = ROOT / 'build/2049LE/obj/NGC/ngcMemcard.o'
        if not built.is_file() or not target.is_file() or not (ROOT.parent / 'rush_d.elf').is_file():
            self.skipTest('requires local memcard object and trusted originals')
        with tempfile.TemporaryDirectory(prefix='fresh_no_output_', dir=TEST_OUTPUT) as directory:
            with patch.object(verify_linked_layout.subprocess, 'run',
                              return_value=SimpleNamespace(returncode=0, stdout='', stderr='')) as run:
                receipt = verify_linked_layout.run_fresh_link(
                    [built], built, target, ROOT.parent / 'rush_d.elf',
                    ROOT / 'orig/2049LE/files/rush_d.dol', ROOT / 'build/2049LE/ldscript.lcf',
                    ROOT / 'build/compilers/GC/2.7/mwldeppc.exe', ROOT / 'build/tools/dtk.exe', Path(directory))
                self.assertEqual(receipt['status'], 'failed')
                self.assertIn('fresh command 0 failed', receipt['failures'][0])
                self.assertEqual(run.call_count, 1)


class RealObjectTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.unit = 'GameSource/src/ll.o'
        cls.target_path = ROOT / 'build/2049LE/obj' / cls.unit
        cls.built_path = ROOT / 'build/2049LE/src' / cls.unit
        if not all(path.exists() for path in (cls.target_path, cls.built_path, ROOT.parent / 'rush_d.elf')):
            raise unittest.SkipTest('requires local ELF and accepted ll objects')

    def test_accepted_object_layout_control(self):
        self.assertEqual(verify_layout.compare_layout(Elf(self.target_path), Elf(self.built_path))[2], [])

    def test_accepted_function_relocation_controls_and_mutations(self):
        cwmatch._orig = None
        built, rels = cwmatch.obj_funcs(self.built_path)
        for name in built:
            self.assertEqual(cwmatch.compare(name, built, rels), 'MATCH', name)
        offset = next(iter(rels))
        kind, target, addend = rels[offset]
        name = next(name for name, (base, code) in built.items() if base <= offset < base + len(code))
        for value in ((11, target, addend), (kind, 'wrong_target', addend), (kind, target, addend + 4)):
            changed = dict(rels)
            changed[offset] = value
            self.assertIn('relocation differences', cwmatch.compare(name, built, changed))

    def test_real_elf_symbol_extent_mutation_rejected(self):
        elf = Elf(self.built_path)
        sym = next(s for s in elf.syms if s['type'] == 2 and s['size'])
        symtab = next(s for s in elf.secs if s['type'] == 2)
        changed = bytearray(elf.d)
        struct.pack_into('>I', changed, symtab['off'] + sym['index'] * 16 + 8, sym['size'] + 4)
        with tempfile.TemporaryDirectory(prefix='verify_elf_', dir=TEST_OUTPUT) as directory:
            path = Path(directory) / 'bad-size.o'
            path.write_bytes(changed)
            self.assertTrue(verify_layout.compare_layout(Elf(self.target_path), Elf(path))[2])

    def test_real_elf_relocation_type_target_addend_mutations_rejected(self):
        elf = Elf(self.built_path)
        relocation = elf.by_name['.rela.text']['off']
        off, info, addend = struct.unpack_from('>IIi', elf.d, relocation)
        wrong_target = next(s['index'] for s in elf.syms
                            if s['name'] and s['index'] != info >> 8 and s['type'] == 2)
        variants = [('type', (off, (info & ~0xFF) | 11, addend)),
                    ('target', (off, (wrong_target << 8) | (info & 0xFF), addend)),
                    ('addend', (off, info, addend + 4))]
        with tempfile.TemporaryDirectory(prefix='verify_elf_', dir=TEST_OUTPUT) as directory:
            for label, values in variants:
                with self.subTest(field=label):
                    changed = bytearray(elf.d)
                    struct.pack_into('>IIi', changed, relocation, *values)
                    path = Path(directory) / f'bad-relocation-{label}.o'
                    path.write_bytes(changed)
                    built, rels = cwmatch.obj_funcs(path)
                    name = next(name for name, (base, code) in built.items()
                                if base <= off < base + len(code))
                    self.assertIn('relocation differences', cwmatch.compare(name, built, rels))

    def test_real_elf_out_of_section_function_rejected(self):
        elf = Elf(self.built_path)
        sym = next(s for s in elf.syms if s['type'] == 2 and s['size'])
        symtab = next(s for s in elf.secs if s['type'] == 2)
        changed = bytearray(elf.d)
        struct.pack_into('>I', changed, symtab['off'] + sym['index'] * 16 + 8, 0x10000)
        with tempfile.TemporaryDirectory(prefix='verify_elf_', dir=TEST_OUTPUT) as directory:
            path = Path(directory) / 'bad-extent.o'
            path.write_bytes(changed)
            with self.assertRaisesRegex(ValueError, 'invalid function extent'):
                cwmatch.obj_funcs(path)

    def test_timer_and_fmath_counter_controls_need_evidence(self):
        for unit, function in [('GameSource/src/timer', 'InitializeTimer'), ('GameSource/src/fmath', 'invsqr')]:
            path = ROOT / 'build/2049LE/src' / (unit + '.o')
            self.assertTrue(path.is_file(), path)
            built, rels = cwmatch.obj_funcs(path)
            self.assertTrue(cwmatch.compare(function, built, rels).startswith('CANDIDATE'), function)

    def test_memcard_strict_failure_has_narrow_linked_evidence_scope(self):
        target = Elf(ROOT / 'build/2049LE/obj/NGC/ngcMemcard.o')
        built = Elf(ROOT / 'build/2049LE/src/NGC/ngcMemcard.o')
        self.assertEqual(len(verify_layout.compare_layout(target, built)[2]), 15)
        failures, shifted, extras = verify_linked_layout.object_exception_scope(target, built)
        self.assertEqual(failures, [])
        self.assertEqual(len(shifted), 15)
        self.assertEqual(extras, ['PS2MCCheckStatus'])


if __name__ == '__main__':
    unittest.main()
