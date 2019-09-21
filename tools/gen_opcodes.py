#!/usr/bin/env python

import imp
import sys
import os
import os.path
import errno
from gen_common import *


_GEN_SKELETON = """\
// THIS FILE IS GENERATED

#include "opcode.hpp"
#include "alu.hpp"
#include "cpu.hpp"

using gem::hexString;

#ifndef NDEBUG
#define UNIMPLEMENTED_OPCODE(WHICH)                                  \\
    do {{                                                             \\
        std::cerr << "unimplemented opcode: 0x" << hexString(WHICH)  \\
                    << " at PC 0x" << hexString(cpu.reg.getPC())     \\
                    << '\\n';                                         \\
        GEM_UNREACHABLE();                                           \\
    }}                                                                \\
    while (false)
#else
#define UNIMPLEMENTED_OPCODE(...) GEM_UNREACHABLE()
#endif

{globals_}

namespace {{
{helper_functions}
{defs}
}}

#if GEM_DEBUG_LOGGING
namespace {{
const char* getOpcodeDescription(const gem::u8 code, const gem::CPU& cpu) {{
    switch (code) {{
        {getters}
    }}
    UNIMPLEMENTED_OPCODE(code);
}}
}}
#endif

gem::DeltaTicks gem::op::runOpcode(const gem::u8 opcode, gem::CPU& cpu) {{
    GEM_DEBUG_LOG("running opcode: " << getOpcodeDescription(opcode, cpu) << " at PC: " << hexString(cpu.reg.getPC()-1)
        << "\\n\\tAF: " << hexString(cpu.reg.getAF())
        << "\\n\\tBC: " << hexString(cpu.reg.getBC())
        << "\\n\\tDE: " << hexString(cpu.reg.getDE())
        << "\\n\\tHL: " << hexString(cpu.reg.getHL())
        << "\\n\\tSP: " << hexString(cpu.reg.getSP())
        << "\\n\\tPC: " << hexString(cpu.reg.getPC()-1)
            << "\\n\\tZ: " << cpu.reg.flags.getZ() 
            << ", N: " << cpu.reg.flags.getN() 
            << ", H: " << cpu.reg.flags.getH() 
            << ", C: " << cpu.reg.flags.getC());
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


def make_getters(ops, two_byte_prefixes):
    one_byte_ops, two_byte_ops = partition_two_byte_ops(ops, two_byte_prefixes)

    def make_one_byte_getter(op):
        return 'case {val}: {{ return "\\"{name}\\" ({val})"; }}'.format(val=op.val, name=op.name)
    one_byte_getters = '\n\t\t'.join(
        make_one_byte_getter(op) for op in one_byte_ops)

    def make_two_byte_getter(item):
        switch_skeleton = """case {prefix}: {{
            const gem::u8 secondByte = cpu.peekPC(); // do NOT increment PC
            switch (secondByte) {{
                {cases}
            }}
            UNIMPLEMENTED_OPCODE(({prefix} << 8) | secondByte);
        }}"""
        cases = ['case {val}: {{ return "\\"{name}\\" ({val})"; }}'.format(val=op.second_byte, name=op.name) for op in sorted(
            item[1], key=lambda op: int(op.second_byte, base=0))]
        return switch_skeleton.format(prefix=item[0], cases='\n\t\t\t\t'.join(cases))
    two_byte_getters = '\n\t\t'.join(make_two_byte_getter(item)
                                     for item in two_byte_ops.items())

    return '{}\n\t\t{}'.format(one_byte_getters, two_byte_getters)


def make_runners(ops, two_byte_prefixes):
    one_byte_ops, two_byte_ops = partition_two_byte_ops(ops, two_byte_prefixes)

    def make_one_byte_runner(op):
        return 'case {val}: {{ return ::run_{name}(cpu); }}'.format(val=op.val, name=sanitize_name(op.name))
    one_byte_runners = '\n\t\t'.join(
        make_one_byte_runner(op) for op in one_byte_ops)

    def make_two_byte_runner(item):
        switch_skeleton = """case {prefix}: {{
            const u8 secondByte = cpu.readPC();
            switch (secondByte) {{
                {cases}
            }}
            UNIMPLEMENTED_OPCODE(({prefix} << 8) | secondByte);
        }}"""
        cases = ['case {val}: {{ return ::run_{name}(cpu); }}'.format(val=op.second_byte, name=sanitize_name(op.name)) for op in sorted(
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
        helper_functions='\n'.join(ops_module.helper_functions),
        globals_='\n'.join(ops_module.globals_))
    with safe_open_w(sys.argv[2]) as f:
        f.write(out)
    print "done"


if __name__ == "__main__":
    main()
