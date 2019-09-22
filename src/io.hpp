#ifndef GEM_IO_HPP
#define GEM_IO_HPP

#include "fwd.hpp"

namespace gem {

struct Mem;

struct IO {
    enum Registers : u16 {
        P1 = 0xFF00,
        SB = 0xFF01,
        SC = 0xFF02,
        DIV = 0xFF04,
        TIMA = 0xFF05,
        TMA = 0xFF06,
        TAC = 0xFF07,
    };

    enum RegisterRange {
        Start = 0xFF00,
        End = 0xFF40,
    };

    void setMem(Mem* const mem) { this->mem = mem; }

    const u8* readOnlyRegisterPtr(const u16 address) const;
    u8* writableRegisterPtr(const u16 address);

    bool consumeWrite(const u16 address, const u8 value);

    void update(Ticks ticks);

   private:
    void updateP1();
    void incTimer();

    Mem* mem = nullptr;
    u8 p1{0xFF};
    u8 sb;
    u8 div = 0x00;
    u8 timer = 0x00;
    u8 tma = 0x00;
    u8 tac = 0x00;

    Ticks timerCounter = 0x00;
    Ticks timerSubCounter = 0x00;
    Ticks divCounter = 0x00;

    std::array<u8, RegisterRange::End - RegisterRange::Start> blob;
};

}  // namespace gem

#endif
