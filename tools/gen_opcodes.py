#!/usr/bin/env python

import imp
import sys
import os
import os.path
import errno


_GEN_SKELETON = """\
// THIS FILE IS GENERATED

#include "opcode.hpp"
#include "cpu.hpp"

#ifndef NDEBUG
#include <cassert>
#include <iostream>
#define UNIMPLEMENTED_OPCODE(WHICH) \\
    do {{ std::cerr << "unimplemented opcode: " << WHICH; assert(false); }} while (false)
#else
#define UNIMPLEMENTED_OPCODE(...)
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
        return """constexpr gem::op::Opcode {sname}{{{val}, {op_count}, {ticks}, "{name}"}};
inline void run_{sname}(gem::CPU& cpu) {{
    (void)cpu;
    using namespace gem;
    {impl}
}}""" \
            .format(sname=sanitize_name(op.name), name=op.name, val=op.val, op_count=op.op_count, ticks=op.ticks, impl=op.implementation)
    return '\n'.join(make_def(op) for op in ops)


def make_getters(ops):
    def make_getter(op):
        return 'case {val}: {{ return ::{name}; }}'.format(val=op.val, name=sanitize_name(op.name))
    return '\n\t\t'.join(make_getter(op) for op in ops)


def make_runners(ops):
    def make_runner(op):
        return 'case {val}: {{ ::run_{name}(cpu); return; }}'.format(val=op.val, name=sanitize_name(op.name))
    return '\n\t\t'.join(make_runner(op) for op in ops)


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


def safe_open_w(path):
    mkdir_p(os.path.dirname(path))
    return open(path, 'w')


def safe_touch(path):
    mkdir_p(os.path.dirname(path))
    open(path, 'w').close()


def is_newer(file_name, than):
    try:
        return os.path.getmtime(file_name) > os.path.getmtime(than)
    except OSError:
        return True


def main():
    if len(sys.argv) != 4:
        print 'usage: {} inputfile outputfile cachefile'.format(sys.argv[0])
        exit(1)

    cache_file = sys.argv[3]
    if not is_newer(__file__, cache_file) and not is_newer(sys.argv[1], cache_file):
        print 'not regenerating opcodes'
        exit(0)
    print 'regenerating opcodes...'
    safe_touch(cache_file)

    ops_module = imp.load_source('opcode', sys.argv[1])
    ops = sorted(list(ops_module.opcodes))
    out = _GEN_SKELETON.format(defs=make_defs(
        ops), getters=make_getters(ops), runners=make_runners(ops))
    with safe_open_w(sys.argv[2]) as f:
        f.write(out)
    print 'wrote opcodes to {}'.format(sys.argv[2])


if __name__ == "__main__":
    main()
