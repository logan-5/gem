#ifndef GEM_CPU_HPP
#define GEM_CPU_HPP

#include "fwd.hpp"
#include "mem.hpp"
#include "opcode.hpp"

#include <cstring>

namespace gem {

struct Registers {
#define REGISTER_PAIR(FIRST, SECOND)                              \
    u8 FIRST, SECOND;                                             \
    u16 get##FIRST##SECOND() const {                              \
        u16 ret;                                                  \
        std::memcpy(&ret, reinterpret_cast<const u8*>(FIRST), 2); \
        return ret;                                               \
    }                                                             \
    void set##FIRST##SECOND(u16 fs) {                             \
        const u8* const p = reinterpret_cast<const u8*>(fs);      \
        std::memcpy(&FIRST, p + 0, 1);                            \
        std::memcpy(&SECOND, p + 1, 1);                           \
    }
    REGISTER_PAIR(A, F)
    REGISTER_PAIR(B, C)
    REGISTER_PAIR(D, E)
    REGISTER_PAIR(H, L)

    u16 SP;
    u16 PC = 0;

#undef REGISTER_PAIR
};

struct FlagRegister {
    bool getZ() const { return get<7>(); }
    void setZ() { set<7>(); }
    void resetZ() { reset<7>(); }

    bool getN() const { return get<6>(); }
    void setN() { set<6>(); }
    void resetN() { reset<6>(); }

    bool getH() const { return get<5>(); }
    void setH() { set<5>(); }
    void resetH() { reset<5>(); }

    bool getC() const { return get<4>(); }
    void setC() { set<4>(); }
    void resetC() { reset<4>(); }

   private:
    template <unsigned Bit>
    bool get() const {
        return (r >> Bit) & 1;
    }
    template <unsigned Bit>
    void set() {
        r |= (1 << Bit);
    }
    template <unsigned Bit>
    void reset() {
        r &= ~(1 << Bit);
    }
    u8 r = 0;
};

struct CPU {
    Registers reg;
    FlagRegister flags;
    Mem bus;
    unsigned long long ticks = 0;

    void execute() { op::runOpcode(*current(), *this); }
    const u8* current() const { return bus.ptr(reg.PC); }
};

}  // namespace gem

#endif
