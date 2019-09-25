#ifndef GEM_CPU_HPP
#define GEM_CPU_HPP

#include "bitwise.hpp"
#include "fwd.hpp"
#include "interrupt.hpp"
#include "mem.hpp"
#include "opcode.hpp"

#include <array>
#include <cstring>

namespace gem {

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

    void set(const u8 val) { r = val & 0xF0; }
    u8 get() const noexcept { return r; }

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

struct Registers {
#define REGISTER_GET_SET(FIRST, SECOND)                       \
   private:                                                   \
    u8 FIRST, SECOND;                                         \
                                                              \
   public:                                                    \
    void set##FIRST(const u8 val) noexcept { FIRST = val; }   \
    u8 get##FIRST() const noexcept { return FIRST; }          \
    u8& get##FIRST##Mut() noexcept { return FIRST; }          \
    void set##SECOND(const u8 val) noexcept { SECOND = val; } \
    u8 get##SECOND() const noexcept { return SECOND; }        \
    u8& get##SECOND##Mut() noexcept { return SECOND; }

   private:
    u8 A;

   public:
    FlagRegister flags;

   public:
    void setA(const u8 val) noexcept { A = val; }
    u8 getA() const noexcept { return A; }
    u8& getAMut() noexcept { return A; }
    void setF(const u8 val) noexcept { flags.set(val); }
    u8 getF() const noexcept { return flags.get(); }

    REGISTER_GET_SET(B, C)
    REGISTER_GET_SET(D, E)
    REGISTER_GET_SET(H, L)

#define REGISTER_PAIR(FIRST, SECOND)                              \
   public:                                                        \
    u16 get##FIRST##SECOND() const noexcept {                     \
        return u16(u16(get##FIRST()) << 8u) | u16(get##SECOND()); \
    }                                                             \
    void set##FIRST##SECOND(const u16 fs) noexcept {              \
        set##FIRST(u8(fs >> 8u));                                 \
        set##SECOND(u8(fs & 0xFF));                               \
    }                                                             \
    void inc##FIRST##SECOND() noexcept {                          \
        set##FIRST##SECOND(get##FIRST##SECOND() + 1);             \
    }                                                             \
    void dec##FIRST##SECOND() noexcept {                          \
        set##FIRST##SECOND(get##FIRST##SECOND() - 1);             \
    }

    REGISTER_PAIR(A, F)
    REGISTER_PAIR(B, C)
    REGISTER_PAIR(D, E)
    REGISTER_PAIR(H, L)

   private:
    u16 SP = 0xFFFE;
    u16 PC = 0x100;

   public:
    u16 getSP() const { return SP; }
    void setSP(const u16 sp) { SP = sp; }
    void incSP() { ++SP; }
    void decSP() { --SP; }

    u16 getPC() const { return PC; }
    void setPC(const u16 pc) { PC = pc; }
    void incPC() { ++PC; }
    void decPC() { --PC; }
    void incPC(const i8 val) { PC += val; }

    friend struct CPU;

#undef REGISTER_PAIR
#undef REGISTER_GET_SET
};

struct CPU {
    explicit CPU(Mem& in_bus) : bus{in_bus} {}

    Registers reg;
    Mem& bus;

    void execute() {
        if (!stopped && !halted) {
            deltaTicks = op::runOpcode(readPC(), *this);
            ticks += deltaTicks;
        }
        if (pendingIME) {
            ime = true;
            pendingIME = false;
        }
    }
    u8 readPC() {
        const auto ret = peekPC();
        if (!haltBug) {
            ++reg.PC;
        } else {
            haltBug = false;
        }
        return ret;
    }
    u16 readPC16() {
        const auto ret = peekPC16();
        reg.PC += 2;
        return ret;
    }
    u8 peekPC() const { return *bus.ptr(reg.PC); }
    u16 peekPC16() const {
        const u8* const ptr = bus.ptr(reg.PC);
        const u16 ret = u16(ptr[0]) | u16(ptr[1] << 8u);
        return ret;
    }

    Ticks getTicks() const { return ticks; }
    DeltaTicks getDeltaTicks() const { return deltaTicks; }

    void ei() { pendingIME = true; }
    void di() { ime = false; }

    void processInterrupts();
    void pushStack(u16 value);
    u16 popStack();
    void handleInterrupt(const u16 destination);
    void returnFromInterrupt();
    u8 getPendingInterrupts() const;
    void halt();
    void stop() {
        stopped = true;  // TODO turn off screen?
    }

   private:
    Ticks ticks = 0;
    DeltaTicks deltaTicks = 0;
    bool ime = false;
    bool pendingIME = false;
    bool halted = false;
    bool haltBug = false;
    bool stopped = false;
};

}  // namespace gem

#endif
