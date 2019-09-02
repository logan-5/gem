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

inline void add8(u8& lhs, u8 rhs, CPU& cpu) {
    detail::add8_impl(lhs, rhs, cpu);
}

inline void adc8(u8& lhs, u8 rhs, CPU& cpu) {
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
    carry ? cpu.flags.resetC() : cpu.flags.setC();
    halfC ? cpu.flags.resetH() : cpu.flags.setH();
}
}  // namespace detail

inline void sub8(u8& lhs, u8 rhs, CPU& cpu) {
    detail::sub8_impl(lhs, rhs, cpu);
}
inline void sbc8(u8& lhs, u8 rhs, CPU& cpu) {
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

inline void swapNybbles(u8& n, CPU& cpu) {
    const u8 upper = n & 0b1111'0000;
    const u8 lower = n & 0b0000'1111;
    const u8 result = u8(upper >> 4) | u8(lower << 4);

    result == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    cpu.flags.resetC();

    n = result;
}

inline void cp(u8 lhs, u8 rhs, CPU& cpu) {
    if (lhs == rhs) {
        cpu.flags.setZ();
        cpu.flags.setH();
        cpu.flags.setC();
    } else {
        cpu.flags.resetZ();

        lhs < rhs ? cpu.flags.resetC() : cpu.flags.setC();
        const u8 noBorrow = lhs ^ rhs;
        const u8 diff = lhs - rhs;
        const u8 borrowResult = noBorrow ^ diff;
        const auto halfCarry = borrowResult & 0b0001'0000;
        halfCarry ? cpu.flags.resetH() : cpu.flags.setH();
    }
    cpu.flags.setN();
}

inline void inc(u8& operand, CPU& cpu) {
    using namespace bitwise;

    const u8 result = operand + 1;
    const auto halfCarry = test<4>(result) ^ test<4>(operand);

    result == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    halfCarry ? cpu.flags.setH() : cpu.flags.resetH();
    // C not affected

    operand = result;
}

inline void dec(u8& operand, CPU& cpu) {
    using namespace bitwise;

    const u8 result = operand - 1;
    const bool halfCarry = test<4>(result) ^ test<4>(operand);

    result == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.setN();
    halfCarry ? cpu.flags.setH() : cpu.flags.resetH();
    // C not affected

    operand = result;
}

[[nodiscard]] inline u16 add16(u16 lhs, u16 rhs, CPU& cpu) {
    const u32 lhs32 = lhs, rhs32 = rhs;
    const u32 result = lhs32 + rhs32;
    const u32 noCarry = lhs32 ^ rhs32;
    const u32 carryResult = result ^ noCarry;

    const auto carry = carryResult & 0b1'0000'0000'0000'0000;
    const auto halfC = carryResult & 0b0'0001'0000'0000'0000;

    // Z not affected
    cpu.flags.resetN();
    carry ? cpu.flags.setC() : cpu.flags.resetC();
    halfC ? cpu.flags.setH() : cpu.flags.resetH();

    return u16(result);
}

[[nodiscard]] inline u16 add16Signed(u16 lhs, u16 rhs, CPU& cpu) {
    const i32 lhs_signed{lhs};
    u32 rhs32 = u32{rhs};
    i32 rhs_signed;
    std::memcpy(&rhs_signed, &rhs32, sizeof rhs_signed);

    const i32 result = lhs_signed + rhs_signed;
    const u32 noCarry = lhs ^ rhs;
    u32 unsignedResult;
    std::memcpy(&unsignedResult, &result, sizeof unsignedResult);
    const u32 carryResult = noCarry ^ unsignedResult;

    const auto carry = carryResult & (1u << 31);
    const auto halfC = carryResult & 0b0'0001'0000'0000'0000;

    cpu.flags.resetZ();
    cpu.flags.resetN();
    if (rhs_signed < 0) {
        carry ? cpu.flags.resetC() : cpu.flags.setC();
        halfC ? cpu.flags.resetH() : cpu.flags.setH();
    } else {
        carry ? cpu.flags.setC() : cpu.flags.resetC();
        halfC ? cpu.flags.setH() : cpu.flags.resetH();
    }

    return u16(result);
}

[[nodiscard]] inline u16 add16Signed8(u16 lhs, u8 rhs, CPU& cpu) {
    return add16Signed(lhs, u16(rhs), cpu);
}

inline void decimalAdjust(u8& operand, CPU& cpu) {
    (void)operand;
    (void)cpu;
    throw;
}

inline void complement(u8& operand, CPU& cpu) {
    const u8 result = ~operand;
    // Z not affected
    cpu.flags.setN();
    cpu.flags.setH();
    // C not affected

    operand = result;
}

inline void rlc(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);

    operand <<= 1u;
    operand |= bit7 ? 1 : 0;

    operand == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    bit7 ? cpu.flags.setC() : cpu.flags.resetC();
}

inline void rl(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);
    const bool oldCarry = cpu.flags.getC();

    operand <<= 1u;
    operand |= oldCarry ? 1u : 0u;

    operand == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    bit7 ? cpu.flags.setC() : cpu.flags.resetC();
}

inline void rrc(u8& operand, CPU& cpu) {
    const bool bit0 = bitwise::test<0>(operand);

    operand >>= 1u;
    operand |= bit0 << 7u;

    operand == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    bit0 ? cpu.flags.setC() : cpu.flags.resetC();
}

inline void rr(u8& operand, CPU& cpu) {
    const bool bit0 = bitwise::test<0>(operand);
    const bool oldCarry = cpu.flags.getC();

    operand >>= 1u;
    operand |= oldCarry << 7u;

    operand == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    bit0 ? cpu.flags.setC() : cpu.flags.resetC();
}

inline void sla(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);
    operand <<= 1u;

    operand == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    bit7 ? cpu.flags.setC() : cpu.flags.resetC();
}

inline void sra(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);
    const bool bit0 = bitwise::test<0>(operand);
    operand >>= 1u;
    operand |= bit7 ? 1u : 0u;

    operand == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    bit0 ? cpu.flags.setC() : cpu.flags.resetC();
}

inline void srl(u8& operand, CPU& cpu) {
    const bool bit0 = bitwise::test<0>(operand);
    operand >>= 1u;
    bitwise::reset<7>(operand);

    operand == 0 ? cpu.flags.setZ() : cpu.flags.resetZ();
    cpu.flags.resetN();
    cpu.flags.resetH();
    bit0 ? cpu.flags.setC() : cpu.flags.resetC();
}

template <unsigned Bit>
inline void bit(u8 operand, CPU& cpu) {
    bitwise::test<Bit>(operand) ? cpu.flags.resetZ() : cpu.flags.setZ();
    cpu.flags.resetN();
    cpu.flags.setH();
    // C not affected
}
template <unsigned Bit>
inline void set(u8& operand, CPU& cpu) {
    (void)cpu;
    bitwise::set<Bit>(operand);
}
template <unsigned Bit>
inline void res(u8& operand, CPU& cpu) {
    (void)cpu;
    bitwise::reset<Bit>(operand);
}

}  // namespace alu
}  // namespace gem

#endif
