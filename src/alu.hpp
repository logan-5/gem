#ifndef GEM_ALU_HPP
#define GEM_ALU_HPP

#include "bitwise.hpp"
#include "cpu.hpp"
#include "fwd.hpp"

namespace gem {
namespace alu {

namespace detail {
inline void add8_impl(u8& lhs, u16 rhs, CPU& cpu) {
    // very helpful:
    // https://stackoverflow.com/questions/42091214/gbz80-adc-instructions-fail-test
    const u16 sum = u16(lhs) + rhs;
    const u16 noCarrySum = u16(lhs) ^ rhs;
    const u16 carryResult = noCarrySum ^ sum;

    const bool carry = carryResult & 0b1'0000'0000;
    const bool halfC = carryResult & 0b0'0001'0000;

    sum == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    carry ? cpu.flags.setC() : cpu.flags.resetC();
    halfC ? cpu.flags.setH() : cpu.flags.resetH();

    lhs = u8(sum);
}
}  // namespace detail

inline void add(u8& lhs, u8 rhs, CPU& cpu) {
    detail::add8_impl(lhs, rhs, cpu);
}

inline void adc(u8& lhs, u8 rhs, CPU& cpu) {
    detail::add8_impl(lhs, u16(rhs) + cpu.flags.getC(), cpu);
}

namespace detail {
inline void sub8_impl(u8& lhs, u16 rhs, CPU& cpu) {
    const u16 diff = u16(lhs) - rhs;
    const u16 noCarryDiff = lhs ^ rhs;
    const u16 carryResult = noCarryDiff ^ diff;

    const bool carry = carryResult & (1 << 15);
    const bool halfC = carryResult & 0b0001'0000;

    diff == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.setN();
    carry ? cpu.flags.setC() : cpu.flags.resetC();
    halfC ? cpu.flags.setH() : cpu.flags.resetH();
}
}  // namespace detail

inline void sub(u8& lhs, u8 rhs, CPU& cpu) {
    detail::sub8_impl(lhs, rhs, cpu);
}
inline void sbc(u8& lhs, u8 rhs, CPU& cpu) {
    detail::sub8_impl(lhs, u16(rhs) + cpu.flags.getC(), cpu);
}

inline void and_(u8& lhs, u8 rhs, CPU& cpu) {
    const bool result = bool(lhs) && bool(rhs);
    result ? cpu.flags.resetZ() : cpu.flags.setZ();
    cpu.flags.resetN();
    cpu.flags.setH();
    cpu.flags.resetC();

    lhs = u8(result);
}

inline void or_(u8& lhs, u8 rhs, CPU& cpu) {
    const bool result = bool(lhs) || bool(rhs);
    result ? cpu.flags.resetZ() : cpu.flags.setZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    cpu.flags.resetC();

    lhs = u8(result);
}

inline void xor_(u8& lhs, u8 rhs, CPU& cpu) {
    const bool result = bool(lhs) ^ bool(rhs);
    result ? cpu.flags.resetZ() : cpu.flags.setZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    cpu.flags.resetC();

    lhs = u8(result);
}

}  // namespace alu
}  // namespace gem

#endif
