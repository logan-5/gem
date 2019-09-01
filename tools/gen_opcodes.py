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

# ifndef NDEBUG
# include <cassert>
# include <iostream>
# include <iomanip>
# define UNIMPLEMENTED_OPCODE(WHICH)                                 \\
    do {{                                                            \\
        std::cerr << "unimplemented opcode: 0x" << std::setfill('0') \\
                    << std::setw(2) << std::hex << int(WHICH)        \\
                    << " at PC 0x" << std::hex << cpu.reg.PC         \\
                    << '\\n';                                        \\
        GEM_UNREACHABLE();                                           \\
    }}                                                               \\
    while (false)
# else
# define UNIMPLEMENTED_OPCODE(...) GEM_UNREACHABLE()
# endif

namespace {{
{helper_functions}
{defs}

#if GEM_DEBUG_LOGGING
std::ostream& operator<<(std::ostream& ostr, const gem::op::Opcode opcode) {{
    using namespace gem;
    ostr << "\\"" << opcode.name << "\\" (0x" << std::hex << int(opcode.val) << ")";
    return ostr;
}}
#endif

}}

gem::Opcode gem::op::getOpcode(const gem::u8 code, gem::CPU& cpu) {{
    switch (code) {{
        {getters}
    }}
    UNIMPLEMENTED_OPCODE(code);
}}
void gem::op::runOpcode(const gem::u8 opcode, gem::CPU& cpu) {{
    GEM_DEBUG_LOG("running opcode: " << getOpcode(opcode, cpu) << " at PC: 0x" << std::hex << cpu.reg.PC);
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
        def make_inc_pc(op):
            # I thought we were going to treat jumps specially, but we're not
            return 'cpu.reg.PC += {};'.format(op.op_count + 1 + (1 if op.second_byte is not None else 0))
        return """constexpr gem::op::Opcode {sname}{{{val}, {op_count}, {ticks}, "{name}"}};
inline void run_{sname}(gem::CPU& cpu) {{
    (void)cpu;
    using namespace gem;
    {impl}
    cpu.ticks += {ticks};
    {inc_pc}
}}""" \
            .format(sname=sanitize_name(op.name), name=op.name, val=op.val, op_count=op.op_count,
                    ticks=op.ticks, impl=op.implementation, inc_pc=make_inc_pc(op))
    return '\n'.join(make_def(op) for op in ops)


def partition_two_byte_ops(ops, two_byte_prefixes):
    one_byte_ops = [op for op in ops if op.val not in two_byte_prefixes]

    two_byte_ops = [op for op in ops if op.val in two_byte_prefixes]
    grouped = {}
    for op in two_byte_ops:
        assert op.second_byte is not None
        grouped.setdefault(op.val, []).append(op)
    return one_byte_ops, grouped


def make_getters(ops, two_byte_prefixes):
    one_byte_ops, two_byte_ops = partition_two_byte_ops(ops, two_byte_prefixes)

    def make_one_byte_getter(op):
        return 'case {val}: {{ return ::{name}; }}'.format(val=op.val, name=sanitize_name(op.name))
    one_byte_getters = '\n\t\t'.join(
        make_one_byte_getter(op) for op in one_byte_ops)

    def make_two_byte_getter(item):
        switch_skeleton = """case {prefix}: {{
            switch (cpu.current()[1]) {{
                {cases}
            }}
            UNIMPLEMENTED_OPCODE(({prefix} << 8) | cpu.current()[1]);
        }}"""
        cases = ['case {val}: {{ return ::{name}; }}'.format(val=op.second_byte, name=sanitize_name(op.name)) for op in sorted(
            item[1], key=lambda op: int(op.second_byte, base=0))]
        return switch_skeleton.format(prefix=item[0], cases='\n\t\t\t\t'.join(cases))
    two_byte_getters = '\n\t\t'.join(make_two_byte_getter(item)
                                     for item in two_byte_ops.items())

    return '{}\n\t\t{}'.format(one_byte_getters, two_byte_getters)


def make_runners(ops, two_byte_prefixes):
    one_byte_ops, two_byte_ops = partition_two_byte_ops(ops, two_byte_prefixes)

    def make_one_byte_runner(op):
        return 'case {val}: {{ ::run_{name}(cpu); return; }}'.format(val=op.val, name=sanitize_name(op.name))
    one_byte_runners = '\n\t\t'.join(
        make_one_byte_runner(op) for op in one_byte_ops)

    def make_two_byte_runner(item):
        switch_skeleton = """case {prefix}: {{
            switch (cpu.current()[1]) {{
                {cases}
            }}
            UNIMPLEMENTED_OPCODE(({prefix} << 8) | cpu.current()[1]);
        }}"""
        cases = ['case {val}: {{ ::run_{name}(cpu); return; }}'.format(val=op.second_byte, name=sanitize_name(op.name)) for op in sorted(
            item[1], key=lambda op: int(op.second_byte, base=0))]
        return switch_skeleton.format(prefix=item[0], cases='\n\t\t\t\t'.join(cases))
    two_byte_runners = '\n\t\t'.join(make_two_byte_runner(item)
                                     for item in two_byte_ops.items())

    return '{}\n\t\t{}'.format(one_byte_runners, two_byte_runners)


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
        helper_functions='\n'.join(ops_module.helper_functions))
    with safe_open_w(sys.argv[2]) as f:
        f.write(out)
    print "done"


if __name__ == "__main__":
    main()
