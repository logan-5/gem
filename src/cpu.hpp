#ifndef GEM_CPU_HPP
#define GEM_CPU_HPP

#include "bitwise.hpp"
#include "fwd.hpp"
#include "mem.hpp"
#include "opcode.hpp"

#include <array>
#include <cstring>

namespace gem {

struct Registers {
#define REGISTER_PAIR(FIRST, SECOND)                  \
    u8 FIRST, SECOND;                                 \
    u16 get##FIRST##SECOND() const {                  \
        return u16(u16(FIRST) << 8u) | u16(SECOND);   \
    }                                                 \
    void set##FIRST##SECOND(const u16 fs) {           \
        FIRST = u8(fs >> 8u);                         \
        SECOND = u8(fs & 0xFF);                       \
    }                                                 \
    void inc##FIRST##SECOND() {                       \
        set##FIRST##SECOND(get##FIRST##SECOND() + 1); \
    }                                                 \
    void dec##FIRST##SECOND() { set##FIRST##SECOND(get##FIRST##SECOND() - 1); }
    REGISTER_PAIR(A, F)
    REGISTER_PAIR(B, C)
    REGISTER_PAIR(D, E)
    REGISTER_PAIR(H, L)

    u16 SP = 0xFFFE;
    u16 PC = 0x100;

    // for symmetry with the 16-bit registers as macro'd out above
    u16 getSP() const { return SP; }
    void setSP(const u16 sp) { SP = sp; }
    void incSP() { ++SP; }
    void decSP() { --SP; }

    u16 getPC() const { return PC; }
    void setPC(const u16 pc) { PC = pc; }
    void incPC() { ++PC; }
    void decPC() { --PC; }

#undef REGISTER_PAIR
};

struct FlagRegister {
    bool getZ() const { return get<7>(); }
    void setZ() { set<7>(); }
    void toggleZ() { toggle<7>(); }
    void resetZ() { reset<7>(); }

    bool getN() const { return get<6>(); }
    void setN() { set<6>(); }
    void toggleN() { toggle<6>(); }
    void resetN() { reset<6>(); }

    bool getH() const { return get<5>(); }
    void setH() { set<5>(); }
    void toggleH() { toggle<5>(); }
    void resetH() { reset<5>(); }

    bool getC() const { return get<4>(); }
    void setC() { set<4>(); }
    void toggleC() { toggle<4>(); }
    void resetC() { reset<4>(); }

   private:
    template <unsigned Bit>
    bool get() const {
        return bitwise::test<Bit>(r);
    }
    template <unsigned Bit>
    void set() {
        bitwise::set<Bit>(r);
    }
    template <unsigned Bit>
    void toggle() {
        bitwise::toggle<Bit>(r);
    }
    template <unsigned Bit>
    void reset() {
        bitwise::reset<Bit>(r);
    }
    u8 r = 0;
};

struct CPU {
    explicit CPU(Mem& in_bus) : bus{in_bus} {}

    Registers reg;
    FlagRegister flags;
    Mem& bus;

    void execute() {
        deltaTicks = op::runOpcode(readPC(), *this);
        ticks += deltaTicks;
    }
    u8 readPC() { return *bus.ptr(reg.PC++); }
    u16 readPC16() {
        const u8* const ptr = bus.ptr(reg.PC);
        const u16 ret = u16(ptr[0]) | u16(ptr[1] << 8u);
        reg.PC += 2;
        return ret;
    }
    u8 peekPC() const { return *bus.ptr(reg.PC); }

    Ticks getTicks() const { return ticks; }
    DeltaTicks getDeltaTicks() const { return deltaTicks; }

   private:
    Ticks ticks = 0;
    DeltaTicks deltaTicks = 0;
};

}  // namespace gem

#endif
