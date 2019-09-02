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
void runOpcode(u8 opcode, CPU& cpu);
inline void runOpcode(Opcode opcode, CPU& cpu) {
    runOpcode(opcode.val, cpu);
}

}  // namespace op

using op::Opcode;

}  // namespace gem

#endif
