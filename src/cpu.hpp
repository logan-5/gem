#ifndef GEM_CPU_HPP
#define GEM_CPU_HPP

#include "bitwise.hpp"
#include "fwd.hpp"
#include "mem.hpp"
#include "opcode.hpp"

#include <cstring>

namespace gem {

struct Registers {
#define REGISTER_PAIR(FIRST, SECOND)                                        \
    u8 FIRST, SECOND;                                                       \
    u16 get##FIRST##SECOND() const {                                        \
        u16 ret;                                                            \
        std::memcpy(&ret, reinterpret_cast<const u8*>(&FIRST), sizeof ret); \
        return ret;                                                         \
    }                                                                       \
    void set##FIRST##SECOND(const u16 fs) {                                 \
        const u8* const p = reinterpret_cast<const u8*>(&fs);               \
        std::memcpy(&FIRST, p + 0, sizeof FIRST);                           \
        std::memcpy(&SECOND, p + 1, sizeof SECOND);                         \
    }                                                                       \
    void inc##FIRST##SECOND() {                                             \
        set##FIRST##SECOND(get##FIRST##SECOND() - 1);                       \
    }                                                                       \
    void dec##FIRST##SECOND() { set##FIRST##SECOND(get##FIRST##SECOND() - 1); }
    REGISTER_PAIR(A, F)
    REGISTER_PAIR(B, C)
    REGISTER_PAIR(D, E)
    REGISTER_PAIR(H, L)

    u16 SP;
    u16 PC = 0;

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
    Registers reg;
    FlagRegister flags;
    Mem bus;
    unsigned long long ticks = 0;

    void execute() { op::runOpcode(readPC(), *this); }
    u8 readPC() { return *bus.ptr(reg.PC++); }
    u16 readPC16() {
        u16 ret;
        std::memcpy(&ret, bus.ptr(reg.PC), sizeof ret);
        reg.PC += 2;
        return ret;
    }
    u8 peekPC() const { return *bus.ptr(reg.PC); }
};

}  // namespace gem

#endif
