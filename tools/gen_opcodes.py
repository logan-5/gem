#!/usr/bin/env python

import imp
import sys
import os
import os.path
import errno
from gen_common import *


_GEN_SKELETON = """\
// THIS FILE IS GENERATED

# include "opcode.hpp"
# include "alu.hpp"
# include "cpu.hpp"

using gem::hexString;

# ifndef NDEBUG
# define UNIMPLEMENTED_OPCODE(WHICH)                                  \\
    do {{                                                             \\
        std::cerr << "unimplemented opcode: 0x" << hexString(WHICH)  \\
                    << " at PC 0x" << hexString(cpu.reg.getPC())     \\
                    << '\\n';                                         \\
        GEM_UNREACHABLE();                                           \\
    }}                                                                \\
    while (false)
# else
# define UNIMPLEMENTED_OPCODE(...) GEM_UNREACHABLE()
# endif

{globals_}

namespace {{
{helper_functions}
{defs}
}}

# if GEM_DEBUG_LOGGING
namespace {{
gem::TinyString<23> getOpcodeDescription(const gem::u8 code, const gem::CPU& cpu) {{
    using namespace gem;
    switch (code) {{
        {getters}
    }}
    UNIMPLEMENTED_OPCODE(code);
}}
}}

namespace {{ bool verbosePrintState = false; }}
void gem::op::enableVerbosePrinting() {{
    ::verbosePrintState = true;
}}
void gem::op::disableVerbosePrinting() {{
    ::verbosePrintState = false;
}}
namespace {{ bool verbosePrint() {{ return ::verbosePrintState; }} }}
# else
namespace {{ constexpr bool verbosePrint() {{ return false; }} }}
# endif

gem::DeltaTicks gem::op::runOpcode(const gem::u8 opcode, gem::CPU& cpu) {{
    if (verbosePrint()) {{
        GEM_DEBUG_LOG("running opcode: " << getOpcodeDescription(opcode, cpu) << " at PC: " << hexString(u16(cpu.reg.getPC()-1))
            << "\\n    AF: " << hexString(cpu.reg.getAF())
            << "\\n    BC: " << hexString(cpu.reg.getBC())
            << "\\n    DE: " << hexString(cpu.reg.getDE())
            << "\\n    HL: " << hexString(cpu.reg.getHL())
            << "\\n    SP: " << hexString(cpu.reg.getSP())
            << "\\n    PC: " << hexString(u16(cpu.reg.getPC()-1))
                << "\\n    Z: " << cpu.reg.flags.getZ()
                << ", N: " << cpu.reg.flags.getN()
                << ", H: " << cpu.reg.flags.getH()
                << ", C: " << cpu.reg.flags.getC());
    }}
    switch (opcode) {{
        {runners}
    }}
    UNIMPLEMENTED_OPCODE(opcode);
}}
"""


def sanitize_name(op_name):
    # TODO this could be optimized?
    split = op_name.split(' ')
    joined = '_'.join(split)
    replaced = joined.replace(',', '')
    replaced = replaced.replace('(', 'addr')
    replaced = replaced.replace(')', '')
    # TODO more replacements as needed
    return 'op_{}'.format(replaced)


def make_defs(ops):
    def make_def(op):
        return """inline gem::DeltaTicks run_{sname}(gem::CPU& cpu) {{
    (void)cpu;
    using namespace gem;
    DeltaTicks ticks = 0;
    {impl}
    ticks += {ticks};
    return ticks;
}}""" \
            .format(sname=sanitize_name(op.name), name=op.name,
                    val=op.val, op_count=op.op_count,
                    ticks=op.ticks, impl=op.implementation)
    return '\n'.join(make_def(op) for op in ops)


def partition_two_byte_ops(ops, two_byte_prefixes):
    one_byte_ops = [op for op in ops if op.val not in two_byte_prefixes]

    two_byte_ops = [op for op in ops if op.val in two_byte_prefixes]
    grouped = {}
    for op in two_byte_ops:
        assert op.second_byte is not None
        grouped.setdefault(op.val, []).append(op)
    return one_byte_ops, grouped


def intersperse(lst, item):
    result = [item] * (len(lst) * 2 - 1)
    result[0::2] = lst
    return result


def flatten(list_of_lists):
    return [item for sublist in list_of_lists for item in sublist]


def collapse_string_literals(list_of_strings):
    ret = []
    i = 0
    while i < len(list_of_strings):
        if list_of_strings[i].startswith('"'):
            collected = [list_of_strings[i][1:-1]]
            i += 1
            while i < len(list_of_strings) and list_of_strings[i].startswith('"'):
                collected.append(list_of_strings[i][1:-1])
                i += 1
            ret.append('tiny_str("{}")'.format(''.join(collected)))
        else:
            ret.append(list_of_strings[i])
            i += 1
    return ret


def get_op_description(op, val):
    two_byte_split = 'nn'
    one_byte_split = 'n'
    chunks = [(c, True)
              for c in op.name.split(two_byte_split)]
    chunks = intersperse(chunks, ('hexString(cpu.peekPC16())', False))

    chunks = [(['"{}"'.format(c)
                for c in chunk[0].split(one_byte_split)], chunk[1]) if chunk[1] else chunk for chunk in chunks]
    chunks = [c for c in flatten([intersperse(c[0], 'hexString(cpu.peekPC())')
                                  if c[1] else [c[0]] for c in chunks]) if c != '""']

    chunks.append('" ({})"'.format(val))
    chunks = ' + '.join(collapse_string_literals(chunks))
    return chunks


def make_getters(ops, two_byte_prefixes):
    one_byte_ops, two_byte_ops = partition_two_byte_ops(ops, two_byte_prefixes)

    def make_one_byte_getter(op):
        return 'case {val}: {{ return {desc}; }}'.format(val=op.val, desc=get_op_description(op, op.val))
    one_byte_getters = ('\n' + ' '*8).join(
        make_one_byte_getter(op) for op in one_byte_ops)

    def make_two_byte_getter(item):
        switch_skeleton = """case {prefix}: {{
            const gem::u8 secondByte = cpu.peekPC(); // do NOT increment PC
            switch (secondByte) {{
                {cases}
            }}
            UNIMPLEMENTED_OPCODE(gem::u16(({prefix} << 8) | secondByte));
        }}"""
        cases = ['case {val}: {{ return {desc}; }}'.format(val=op.second_byte, desc=get_op_description(op, op.second_byte)) for op in sorted(
            item[1], key=lambda op: int(op.second_byte, base=0))]
        return switch_skeleton.format(prefix=item[0], cases=('\n' + ' '*16).join(cases))
    two_byte_getters = ('\n' + ' '*8).join(make_two_byte_getter(item)
                                           for item in two_byte_ops.items())

    return '{one}\n{ws}{two}'.format(one=one_byte_getters, ws=' '*8, two=two_byte_getters)


def make_runners(ops, two_byte_prefixes):
    one_byte_ops, two_byte_ops = partition_two_byte_ops(ops, two_byte_prefixes)

    def make_one_byte_runner(op):
        return 'case {val}: {{ return ::run_{name}(cpu); }}'.format(val=op.val, name=sanitize_name(op.name))
    one_byte_runners = ('\n' + ' '*8).join(
        make_one_byte_runner(op) for op in one_byte_ops)

    def make_two_byte_runner(item):
        switch_skeleton = """case {prefix}: {{
            const u8 secondByte = cpu.readPC();
            switch (secondByte) {{
                {cases}
            }}
            UNIMPLEMENTED_OPCODE(gem::u16(({prefix} << 8) | secondByte));
        }}"""
        cases = ['case {val}: {{ return ::run_{name}(cpu); }}'.format(val=op.second_byte, name=sanitize_name(op.name)) for op in sorted(
            item[1], key=lambda op: int(op.second_byte, base=0))]
        return switch_skeleton.format(prefix=item[0], cases=('\n' + ' '*16).join(cases))
    two_byte_runners = ('\n' + ' '*8).join(make_two_byte_runner(item)
                                           for item in two_byte_ops.items())

    return '{one}\n{ws}{two}'.format(one=one_byte_runners, ws=' '*8, two=two_byte_runners)


def main():
    if len(sys.argv) != 3:
        print 'usage: {} inputfile outputfile'.format(sys.argv[0])
        exit(1)

    print "generating '{}' from '{}'".format(sys.argv[2], sys.argv[1])
    ops_module = imp.load_source('opcode', sys.argv[1])
    ops = sorted(list(ops_module.opcodes), key=lambda op: int(op.val, base=0))

    out = _GEN_SKELETON.format(defs=make_defs(
        ops), getters=make_getters(ops, ops_module.two_byte_prefixes),
        runners=make_runners(ops, ops_module.two_byte_prefixes),
        helper_functions='\n'.join(ops_module.helper_functions),
        globals_='\n'.join(ops_module.globals_))
    with safe_open_w(sys.argv[2]) as f:
        f.write(out)
    print "done"


if __name__ == "__main__":
    main()
