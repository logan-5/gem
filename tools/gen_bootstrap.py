#!/usr/bin/env python

import sys
import os
import os.path

from gen_common import *

_SKELETON = """\
#include "bootstrap.hpp"

#include <array>
#include <limits>
#include <vector>

namespace {{
constexpr std::array<gem::u8, {len}> data {{
    {data}
}};
}}

std::vector<gem::u8> gem::loadBootstrapROM() {{
    std::vector<gem::u8> bytes;
    bytes.reserve(std::numeric_limits<u16>::max()); // optimization for when this is loaded by Mem

    bytes.insert(bytes.end(), ::data.begin(), ::data.end());
    return bytes;
}}
"""


def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in xrange(0, len(l), n):
        yield l[i:i + n]


def main():
    if len(sys.argv) != 3:
        print 'usage: {} inputfile outputfile'.format(sys.argv[0])
        exit(1)

    print "generating '{}' from '{}'".format(sys.argv[2], sys.argv[1])

    buffer = open(sys.argv[1], r'r').read()
    as_ints = [hex(ord(byte)) for byte in buffer]
    out = _SKELETON.format(data=',\n\t'.join(',\t'.join(chunk)
                                             for chunk in chunks(as_ints, 8)), len=len(as_ints))

    with safe_open_w(sys.argv[2]) as f:
        f.write(out)
    print 'done'


if __name__ == "__main__":
    main()
