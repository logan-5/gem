#ifndef GEM_BITWISE_HPP
#define GEM_BITWISE_HPP

#include "fwd.hpp"

namespace gem {
namespace bitwise {
inline constexpr void set(u8& b, unsigned bit) {
    b |= 1 << bit;
}
template <unsigned Bit>
void set(u8& b) {
    set(b, Bit);
}
inline constexpr void reset(u8& b, unsigned bit) {
    b &= ~(1 << bit);
}
template <unsigned Bit>
void reset(u8& b) {
    reset(b, Bit);
}
inline constexpr void toggle(u8& b, unsigned bit) {
    b ^= 1 << bit;
}
template <unsigned Bit>
void toggle(u8& b) {
    toggle(b, Bit);
}
inline constexpr bool test(const u8 b, unsigned bit) {
    return (b >> bit) & 1;
}
template <unsigned Bit>
constexpr bool test(const u8 b) {
    return test(b, Bit);
}
}  // namespace bitwise
}  // namespace gem

#endif
