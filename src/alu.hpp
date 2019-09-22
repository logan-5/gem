#ifndef GEM_ALU_HPP
#define GEM_ALU_HPP

#include "bitwise.hpp"
#include "cpu.hpp"
#include "fwd.hpp"

namespace gem {
namespace alu {

namespace detail {
[[nodiscard]] constexpr inline u16 addIntermediate(const u16 lhs,
                                                   const u16 rhs,
                                                   bool& carry,
                                                   bool& halfC) {
    const u16 sum = lhs + rhs;
    const u16 noCarrySum = lhs ^ rhs;
    const u16 carryResult = noCarrySum ^ sum;

    carry = carry || (carryResult & 0b1'0000'0000);
    halfC = halfC || (carryResult & 0b0'0001'0000);

    return sum;
}
inline void setAddFlags(const u8 result,
                        const bool carry,
                        const bool halfC,
                        CPU& cpu) {
    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    carry ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
    halfC ? cpu.reg.flags.setH() : cpu.reg.flags.resetH();
}
}  // namespace detail

inline void add8(u8& lhs, u8 rhs, CPU& cpu) {
    bool carry = false, halfC = false;
    const u16 sum = detail::addIntermediate(lhs, rhs, carry, halfC);

    const u8 ret = u8(sum & 0xFF);
    detail::setAddFlags(ret, carry, halfC, cpu);

    lhs = ret;
}

inline void adc8(u8& lhs, u8 rhs, CPU& cpu) {
    bool carry = false, halfC = false;
    const u16 rhs16 = detail::addIntermediate(
          rhs, cpu.reg.flags.getC() ? 1u : 0u, carry, halfC);
    const u16 ret16 = detail::addIntermediate(u16(lhs), rhs16, carry, halfC);

    const u8 ret = u8(ret16 & 0xFF);
    detail::setAddFlags(ret, carry, halfC, cpu);

    lhs = ret;
}

namespace detail {
[[nodiscard]] constexpr inline u16 subIntermediate(const u16 lhs,
                                                   const u16 rhs,
                                                   bool& carry,
                                                   bool& halfC) {
    const u16 diff = lhs - rhs;
    const u16 noCarryDiff = lhs ^ rhs;
    const u16 carryResult = noCarryDiff ^ diff;

    carry = carry || carryResult & (1 << 15);
    halfC = halfC || carryResult & 0b0001'0000;

    return diff;
}
inline void setSubFlags(const u8 result,
                        const bool carry,
                        const bool halfC,
                        CPU& cpu) {
    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.setN();
    carry ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
    halfC ? cpu.reg.flags.setH() : cpu.reg.flags.resetH();
}
}  // namespace detail

inline void sub8(u8& lhs, u8 rhs, CPU& cpu) {
    bool carry = false, halfC = false;
    const u16 diff = detail::subIntermediate(lhs, rhs, carry, halfC);

    const u8 ret = u8(diff & 0xff);
    detail::setSubFlags(ret, carry, halfC, cpu);

    lhs = ret;
}
inline void sbc8(u8& lhs, u8 rhs, CPU& cpu) {
    bool carry = false, halfC = false;
    const u16 rhs16 = detail::addIntermediate(
          rhs, cpu.reg.flags.getC() ? 1u : 0u, carry, halfC);
    const u16 diff = detail::subIntermediate(u16(lhs), rhs16, carry, halfC);

    const u8 ret = u8(diff & 0xff);
    detail::setSubFlags(ret, carry, halfC, cpu);

    lhs = ret;
}

inline void and_(u8& lhs, u8 rhs, CPU& cpu) {
    const u8 result = lhs & rhs;
    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.setH();
    cpu.reg.flags.resetC();

    lhs = result;
}

inline void or_(u8& lhs, u8 rhs, CPU& cpu) {
    const u8 result = lhs | rhs;
    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    cpu.reg.flags.resetC();

    lhs = result;
}

inline void xor_(u8& lhs, u8 rhs, CPU& cpu) {
    const u8 result = lhs ^ rhs;
    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    cpu.reg.flags.resetC();

    lhs = result;
}

inline void swapNybbles(u8& n, CPU& cpu) {
    const u8 upper = n & 0b1111'0000;
    const u8 lower = n & 0b0000'1111;
    const u8 result = u8(upper >> 4) | u8(lower << 4);

    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    cpu.reg.flags.resetC();

    n = result;
}

inline void cp(u8 lhs, u8 rhs, CPU& cpu) {
    if (lhs == rhs) {
        cpu.reg.flags.setZ();
        cpu.reg.flags.resetH();
        cpu.reg.flags.resetC();
    } else {
        cpu.reg.flags.resetZ();

        lhs < rhs ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
        const u8 noBorrow = lhs ^ rhs;
        const u8 diff = lhs - rhs;
        const u8 borrowResult = noBorrow ^ diff;
        const auto halfCarry = borrowResult & 0b0001'0000;
        halfCarry ? cpu.reg.flags.setH() : cpu.reg.flags.resetH();
    }
    cpu.reg.flags.setN();
}

inline void inc(u8& operand, CPU& cpu) {
    using namespace bitwise;

    const u8 result = operand + 1;
    const auto halfCarry = test<4>(result) ^ test<4>(operand);

    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    halfCarry ? cpu.reg.flags.setH() : cpu.reg.flags.resetH();
    // C not affected

    operand = result;
}

inline void dec(u8& operand, CPU& cpu) {
    using namespace bitwise;

    const u8 result = operand - 1;
    const bool halfCarry = test<4>(result) ^ test<4>(operand);

    result == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.setN();
    halfCarry ? cpu.reg.flags.setH() : cpu.reg.flags.resetH();
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
    cpu.reg.flags.resetN();
    carry ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
    halfC ? cpu.reg.flags.setH() : cpu.reg.flags.resetH();

    return u16(result);
}

[[nodiscard]] inline u16 add16Signed8(u16 lhs, u8 rhs, CPU& cpu) {
    i8 signed_rhs;
    std::memcpy(&signed_rhs, &rhs, sizeof signed_rhs);
    const u16 result = u16(lhs + signed_rhs);

    // this instruction sets flags weirdly, based on adding _unsigned_ rhs
    // http://forums.nesdev.com/viewtopic.php?p=42138
    const u16 lhs8 = lhs & 0xFF;
    const u16 unsignedResult = lhs8 + u16(rhs);
    const u16 noCarry = lhs8 ^ u16(rhs);
    const u16 carryResult = unsignedResult ^ noCarry;
    const bool carry = carryResult & 0b1'0000'0000;
    const bool halfC = carryResult & 0b0'0001'0000;

    cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    carry ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
    halfC ? cpu.reg.flags.setH() : cpu.reg.flags.resetH();

    return result;
}

inline void decimalAdjust(u8& operand, CPU& cpu) {
    // in theory this would've been fun to figure out how to implement alone,
    // but it's so full of poorly-documented gotchas that it would have been a
    // huge and depressing time waste. this implementation is heavily
    // paraphrased from here: https://ehaskins.com/2018-01-30%20Z80%20DAA/
    u8 correction = 0;
    bool setC = false;

    if (cpu.reg.flags.getH() ||
        (!cpu.reg.flags.getN() && (operand & 0xF) > 9)) {
        correction |= 6;
    }
    if (cpu.reg.flags.getC() || (!cpu.reg.flags.getN() && operand > 0x99)) {
        correction |= 6 << 4;
        setC = true;
    }
    operand += cpu.reg.flags.getN() ? -correction : correction;

    operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetH();
    setC ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

inline void complement(u8& operand, CPU& cpu) {
    const u8 result = ~operand;
    // Z not affected
    cpu.reg.flags.setN();
    cpu.reg.flags.setH();
    // C not affected

    operand = result;
}

template <bool SetZ>
inline void rlc(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);

    operand <<= 1u;
    bit7 ? bitwise::set<0>(operand) : bitwise::reset<0>(operand);

    if constexpr (SetZ) {
        operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    } else {
        cpu.reg.flags.resetZ();
    }
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    bit7 ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

template <bool SetZ>
inline void rl(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);
    const bool oldCarry = cpu.reg.flags.getC();

    operand <<= 1u;
    oldCarry ? bitwise::set<0>(operand) : bitwise::reset<0>(operand);

    if constexpr (SetZ) {
        operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    } else {
        cpu.reg.flags.resetZ();
    }
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    bit7 ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

template <bool SetZ>
inline void rrc(u8& operand, CPU& cpu) {
    const bool bit0 = bitwise::test<0>(operand);

    operand >>= 1u;
    bit0 ? bitwise::set<7>(operand) : bitwise::reset<7>(operand);

    if constexpr (SetZ) {
        operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    } else {
        cpu.reg.flags.resetZ();
    }
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    bit0 ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

template <bool SetZ>
inline void rr(u8& operand, CPU& cpu) {
    const bool bit0 = bitwise::test<0>(operand);
    const bool oldCarry = cpu.reg.flags.getC();

    operand >>= 1u;
    oldCarry ? bitwise::set<7>(operand) : bitwise::reset<7>(operand);

    if constexpr (SetZ) {
        operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    } else {
        cpu.reg.flags.resetZ();
    }
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    bit0 ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

inline void sla(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);
    operand <<= 1u;

    operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    bit7 ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

inline void sra(u8& operand, CPU& cpu) {
    const bool bit7 = bitwise::test<7>(operand);
    const bool bit0 = bitwise::test<0>(operand);
    operand >>= 1u;
    bit7 ? bitwise::set<7>(operand) : bitwise::reset<7>(operand);

    operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    bit0 ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

inline void srl(u8& operand, CPU& cpu) {
    const bool bit0 = bitwise::test<0>(operand);
    operand >>= 1u;
    bitwise::reset<7>(operand);

    operand == 0 ? cpu.reg.flags.setZ() : cpu.reg.flags.resetZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.resetH();
    bit0 ? cpu.reg.flags.setC() : cpu.reg.flags.resetC();
}

template <unsigned Bit>
inline void bit(u8 operand, CPU& cpu) {
    bitwise::test<Bit>(operand) ? cpu.reg.flags.resetZ() : cpu.reg.flags.setZ();
    cpu.reg.flags.resetN();
    cpu.reg.flags.setH();
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
