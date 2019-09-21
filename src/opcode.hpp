#ifndef GEM_OPCODE_HPP
#define GEM_OPCODE_HPP

#include "fwd.hpp"

namespace gem {

struct CPU;

namespace op {
// this implementation is generated
DeltaTicks runOpcode(u8 opcode, CPU& cpu);

#if GEM_DEBUG_LOGGING
void enableVerbosePrinting();
void disableVerbosePrinting();
#else
inline void enableVerbosePrinting() {}
inline void disableVerbosePrinting() {}
#endif

}  // namespace op

}  // namespace gem

#endif
