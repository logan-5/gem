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
#include "cpu.hpp"

#ifndef NDEBUG
#include <cassert>
#include <iostream>
#include <iomanip>
#define UNIMPLEMENTED_OPCODE(WHICH)                                  \\
    do {{                                                            \\
        std::cerr << "unimplemented opcode: 0x" << std::setfill('0') \\
                    << std::setw(2) << std::hex << int(WHICH)        \\
                    << '\\n';                                        \\
        GEM_UNREACHABLE();                                           \\
    }}                                                               \\
    while (false)
#else
#define UNIMPLEMENTED_OPCODE(...) GEM_UNREACHABLE()
#endif

namespace {{
{defs}
}}

gem::Opcode gem::op::getOpcode(const gem::u8 code) {{
    switch (code) {{
        {getters}
    }}
    UNIMPLEMENTED_OPCODE(code);
}}
void gem::op::runOpcode(const gem::u8 opcode, gem::CPU& cpu) {{
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
            return '' if op.is_jump else 'cpu.reg.PC += {};'.format(op.op_count + 1)
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


def make_getters(ops):
    def make_getter(op):
        return 'case {val}: {{ return ::{name}; }}'.format(val=op.val, name=sanitize_name(op.name))
    return '\n\t\t'.join(make_getter(op) for op in ops)


def make_runners(ops):
    def make_runner(op):
        return 'case {val}: {{ ::run_{name}(cpu); return; }}'.format(val=op.val, name=sanitize_name(op.name))
    return '\n\t\t'.join(make_runner(op) for op in ops)


def main():
    if len(sys.argv) != 3:
        print 'usage: {} inputfile outputfile'.format(sys.argv[0])
        exit(1)

    print "generating '{}' from '{}'".format(sys.argv[2], sys.argv[1])
    ops_module = imp.load_source('opcode', sys.argv[1])
    ops = sorted(list(ops_module.opcodes), key=lambda op: int(op.val, base=0))
    out = _GEN_SKELETON.format(defs=make_defs(
        ops), getters=make_getters(ops), runners=make_runners(ops))
    with safe_open_w(sys.argv[2]) as f:
        f.write(out)
    print "done"


if __name__ == "__main__":
    main()
