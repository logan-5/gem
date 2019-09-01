#ifndef GEM_BITWISE_HPP
#define GEM_BITWISE_HPP

#include "fwd.hpp"

namespace gem {
namespace bitwise {
template <unsigned Bit>
void set(u8& b) {
    b |= 1 << Bit;
}
template <unsigned Bit>
void reset(u8& b) {
    b &= ~(1 << Bit);
}
template <unsigned Bit>
bool test(const u8 b) {
    return (b >> Bit) & 1;
}
}  // namespace bitwise
}  // namespace gem

#endif
