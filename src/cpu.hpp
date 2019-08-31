#ifndef GEM_CPU_HPP
#define GEM_CPU_HPP

#include "fwd.hpp"
#include "mem.hpp"

#include <cstring>

namespace gem {

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
    u8 A;
    u8 F;
    u16 getAF() const {
        u16 ret;
        std::memcpy(&ret, reinterpret_cast<const u8*>(A), 2);
        return ret;
    }
    void setAF(u16 af) {
        auto* p = reinterpret_cast<const u8*>(af);
        std::memcpy(&A, p + 0, 1);
        std::memcpy(&F, p + 1, 1);
    }

    u8 B;
    u8 C;
    u16 getBC() const {
        u16 ret;
        std::memcpy(&ret, reinterpret_cast<const u8*>(B), 2);
        return ret;
    }
    void setBC(u16 bc) {
        auto* p = reinterpret_cast<const u8*>(bc);
        std::memcpy(&B, p + 0, 1);
        std::memcpy(&C, p + 1, 1);
    }

    u8 D;
    u8 E;
    u16 getDE() const {
        u16 ret;
        std::memcpy(&ret, reinterpret_cast<const u8*>(D), 2);
        return ret;
    }
    void setDE(u16 de) {
        auto* p = reinterpret_cast<const u8*>(de);
        std::memcpy(&D, p + 0, 1);
        std::memcpy(&E, p + 1, 1);
    }

    u8 H;
    u8 L;
    u16 getHL() const {
        u16 ret;
        std::memcpy(&ret, reinterpret_cast<const u8*>(H), 2);
        return ret;
    }
    void setHL(u16 hl) {
        auto* p = reinterpret_cast<const u8*>(hl);
        std::memcpy(&H, p + 0, 1);
        std::memcpy(&L, p + 1, 1);
    }

    u16 SP;
    u16 PC;

    FlagRegister flags;

    Mem mem;

    const u8* current() const;
};

}  // namespace gem

#endif
