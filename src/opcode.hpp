#ifndef GEM_OPCODE_HPP
#define GEM_OPCODE_HPP

#include "fwd.hpp"

namespace gem {

struct CPU;

namespace op {

struct Opcode {
    u8 val;
    u8 operandCount;
    u8 ticks;
    const char* name;
};

// these implementations are generated
Opcode getOpcode(u8 code, const CPU& cpu);
DeltaTicks runOpcode(u8 opcode, CPU& cpu);
inline DeltaTicks runOpcode(Opcode opcode, CPU& cpu) {
    return runOpcode(opcode.val, cpu);
}

}  // namespace op

using op::Opcode;

}  // namespace gem

#endif
