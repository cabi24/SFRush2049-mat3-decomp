"""Small synthetic DWARF/dtk fixtures; no game files or build output needed.

Run from decomp: python -B -m unittest discover -s tools/tests -p test_skel.py -v
"""
import contextlib
import io
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import skel


def function(name, lo=0x80001000, static=False, decl=None, locals_=None, params=None):
    return dict(name=name, lo=lo, hi=lo + 4, stripped=not lo, static=static,
                decl=decl or 'void ' + name + '(void)', locals=locals_ or [], params=params or [])


def variable(name, decl, addr, static=False):
    return dict(name=name, decl=decl, addr=addr, static=static, size=4)


def local(decl, reg='r31', addr=None):
    return dict(decl=decl, reg=reg, static_addr=addr)


def obj(name, addr, values, section='.data'):
    return (f'.section {section}\n# {section}:0x0 | {addr:#010x} | size: 0x4\n.obj {name}, local\n'
            + values + f'\n.endobj {name}\n')


def body(name, lo=0x80001000, references=''):
    marker = f'// Range: {lo:#010x} -> {lo + 4:#010x}' if lo else '// Erased'
    return f'{marker}\nvoid {name}(void) {{\n{references}\n}}\n'


def render(variables, functions, dwarf, assembly, overrides=None, data_only=False):
    cu = dict(path=r'D:\source\fixture.c', variables=variables, functions=functions)
    with patch.object(skel, 'dwarf_section', return_value=dwarf), \
            patch.object(skel, 'asm_text', return_value=assembly):
        values, objs = skel.initializer_values('fixture')
    output = io.StringIO()
    with contextlib.redirect_stdout(output):
        skel.emit('fixture', cu, values, objs, assembly, dwarf, overrides, data_only)
    return output.getvalue(), values


class SkeletonTests(unittest.TestCase):
    def test_baseline_file_data_and_deferred_reverse_order(self):
        variables = [variable('counter', 'int counter', 0x80100000),
                     variable('low', 'int low', 0x80200000, True),
                     variable('high', 'int high', 0x80200004, True)]
        dwarf = 'int counter; // size: 0x4, address: 0x80100000\n'
        dwarf += 'static int low; // size: 0x4, address: 0x80200000\n'
        dwarf += 'static int high; // size: 0x4, address: 0x80200004\n'
        assembly = obj('counter', 0x80100000, '.4byte 7')
        assembly += obj('low', 0x80200000, '.skip 4', '.bss')
        assembly += obj('high', 0x80200004, '.skip 4', '.bss')
        output, _ = render(variables, [], dwarf, assembly)
        self.assertEqual(output,
                         '/* fixture.c - TODO: describe. Status: in progress.\n'
                         '   Original: D:\\source\\fixture.c; assembly: build/2049LE/asm/fixture.s */\n'
                         '#include "rush/rush_protos.h"\n\n'
                         'int counter = 7;\nstatic int high;\nstatic int low;\n\n')

    def test_rodata_local_static_uses_address_suffix_and_owner(self):
        variables = [variable('gravity', 'float gravity[3]', 0x80100000, True)]
        dwarf = body('Before')
        dwarf += 'static float gravity[3]; // size: 0xC, address: 0x80100000\n'
        dwarf += body('Fall', 0x80001004, '    // -> static float gravity[3];')
        assembly = obj('gravity$15', 0x80100000, '.float 0\n.float -0.25\n.float 0', '.rodata')
        output, values = render(variables, [function('Before'), function('Fall', 0x80001004)], dwarf, assembly)
        self.assertEqual(values['gravity$15'], ['0.0f', '-0.25f', '0.0f'])
        self.assertIn('void Fall(void)\n{\n    static const float gravity[3] = { 0.0f, -0.25f, 0.0f };', output)
        self.assertEqual(output.count('static const float gravity[3]'), 1)
        self.assertNotIn('TODO initializer', output)

    def test_repeated_local_names_keep_distinct_initializers_and_owners(self):
        variables = [variable('at', 'float at[3]', 0x80100000, True),
                     variable('at', 'float at[3]', 0x8010000C, True)]
        dwarf = 'static float at[3]; // size: 0xC, address: 0x80100000\n' + body('First')
        dwarf += 'static float at[3]; // size: 0xC, address: 0x8010000C\n' + body('Second', 0x80001004)
        assembly = obj('at$10', 0x80100000, '.float 0\n.float 0\n.float 1')
        assembly += obj('at$20', 0x8010000C, '.float 0\n.float 1\n.float 0')
        output, values = render(variables, [function('First'), function('Second', 0x80001004)], dwarf, assembly)
        self.assertEqual(set(values), {'at$10', 'at$20'})
        self.assertIn('void First(void)\n{\n    static float at[3] = { 0.0f, 0.0f, 1.0f };', output)
        self.assertIn('void Second(void)\n{\n    static float at[3] = { 0.0f, 1.0f, 0.0f };', output)

    def test_plain_mid_file_global_stays_outside_function(self):
        dwarf = body('Before', references='    // -> static int middle;')
        dwarf += 'static int middle; // size: 0x4, address: 0x80100000\n' + body('After', 0x80001004)
        output, _ = render([variable('middle', 'int middle', 0x80100000, True)],
                           [function('Before'), function('After', 0x80001004)], dwarf,
                           obj('middle', 0x80100000, '.4byte 5'))
        self.assertIn('}\n\nstatic int middle = 5;\n\n/* 0x80001004', output)
        self.assertNotIn('    static int middle', output)

    def test_multiline_return_type_is_an_owner_boundary(self):
        dwarf = 'static int x; // size: 0x4, address: 0x80100000\n'
        dwarf += '''// Range: 0x80001000 -> 0x80001004
static // total size: 0x4
struct {
    int member; // offset 0x0, size 0x4
} *First(void) {
    // -> void Second(void);
}
'''
        dwarf += body('Second', 0x80001004)
        output, _ = render([variable('x', 'int x', 0x80100000, True)],
                           [function('First'), function('Second', 0x80001004)], dwarf,
                           obj('x$1', 0x80100000, '.4byte 9'))
        self.assertIn('void First(void)\n{\n    static int x = 9;', output)
        self.assertNotIn('void Second(void)\n{\n    static', output)

    def test_global_before_stripped_function_is_not_lost(self):
        dwarf = body('Before') + 'static int middle; // size: 0x4, address: 0x80100000\n'
        dwarf += body('Gone', 0) + body('After', 0x80001004)
        output, _ = render([variable('middle', 'int middle', 0x80100000, True)],
                           [function('Before'), function('Gone', 0), function('After', 0x80001004)],
                           dwarf, obj('middle', 0x80100000, '.4byte 5'))
        self.assertIn('}\n\nstatic int middle = 5;\n\n/* 0x80001004', output)

    def test_references_and_addressless_declarations_cannot_move_or_define_data(self):
        dwarf = 'int top; // size: 0x4, address: 0x80100000\n'
        dwarf += body('Only', references='    // -> int top;\n    // -> int reference;')
        output, _ = render([variable('top', 'int top', 0x80100000),
                            variable('reference', 'int reference', None)], [function('Only')], dwarf,
                           obj('top', 0x80100000, '.4byte 1') + obj('reference', 0x80100004, '.4byte 2'))
        self.assertLess(output.index('int top = 1;'), output.index('void Only(void)'))
        self.assertNotIn('int reference', output)

    def test_ambiguous_or_wrong_address_is_never_guessed(self):
        objs = {'x$1': (), 'x$2': (), 'x': ()}
        self.assertIsNone(skel.object_symbol('x', 1, objs, {'x$1': 1, 'x$2': 1, 'x': 2}))
        self.assertIsNone(skel.object_symbol('x', 3, objs, {'x$1': 1, 'x$2': 2, 'x': 4}))
        self.assertEqual(skel.object_symbol('x', 3, {'x': ()}, {}), 'x')

    def test_duplicate_block_locals_keep_evidence_without_redeclarations(self):
        f = function('Only', locals_=[local('int p'), local('int p'), local('float p'),
                                     local('int i'), local('int i'), local('int arg')],
                     params=[dict(decl='int arg', reg='r3')], decl='void Only(int arg)')
        output, _ = render([], [f], body('Only'), '')
        self.assertEqual(output.count('    int p;'), 1)
        self.assertEqual(output.count('    int i;'), 1)
        self.assertNotIn('    float p;', output)
        self.assertEqual(output.count('TODO restore DWARF block scope'), 4)

    def test_override_wins_for_definition_and_static_forward_prototype(self):
        f = function('Only', static=True, decl='void Only(int x)',
                     params=[dict(decl='int x', reg='r3')], locals_=[local('int added')])
        override = {'prototypes': {'Only': 'void Only(int x, float (*matrix)[4], int added);'}}
        output, _ = render([], [f], body('Only'), '', override)
        self.assertEqual(output.count('static void Only(int x, float (*matrix)[4], int added)'), 2)
        self.assertNotIn('    int added;', output)
        self.assertIn('TODO restore DWARF block scope: int added;', output)

    def test_qualifier_not_duplicated_and_unknown_layout_stays_todo(self):
        output, _ = render([variable('x', 'const int x', 0x80100000)], [],
                           'int x; // size: 0x4, address: 0x80100000\n',
                           obj('x', 0x80100000, '.4byte 8', '.rodata'))
        self.assertNotIn('const const', output)
        self.assertIn('const int x =', output)
        output, _ = render([variable('x', 'Unknown x', 0x80100000)], [],
                           'Unknown x; // size: 0x4, address: 0x80100000\n',
                           obj('x', 0x80100000, '.4byte 8'))
        self.assertIn('/* TODO initializer: Unknown x (.data) */', output)

    def test_function_metadata_local_bss_is_emitted(self):
        f = function('Only', locals_=[local('int count', addr=0x80200000)])
        output, _ = render([], [f], body('Only'), obj('count$10', 0x80200000, '.skip 4', '.sbss'))
        self.assertIn('    static int count;', output)

    def test_data_only_does_not_hoist_local_static(self):
        dwarf = 'static int count; // size: 0x4, address: 0x80200000\n' + body('Only')
        output, _ = render([variable('count', 'int count', 0x80200000, True)], [function('Only')],
                           dwarf, obj('count$10', 0x80200000, '.skip 4', '.sbss'), data_only=True)
        self.assertNotIn('static int count', output)
        self.assertNotIn('void Only', output)

    def test_stripped_owner_and_missing_owner_are_explicit(self):
        variables = [variable('x', 'int x', 0x80100000, True)]
        assembly = obj('x$10', 0x80100000, '.4byte 2')
        dwarf = 'static int x; // size: 0x4, address: 0x80100000\n'
        output, _ = render(variables, [function('Gone', 0)], dwarf + body('Gone', 0), assembly)
        self.assertIn('local statics of stripped Gone require a placeholder body', output)
        output, _ = render(variables, [], dwarf, assembly)
        self.assertIn('TODO local-static scope: x$10', output)
        self.assertNotIn('\nstatic int x =', output)


if __name__ == '__main__':
    unittest.main()
