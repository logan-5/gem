#ifndef GEM_INTERRUPT_HPP
#define GEM_INTERRUPT_HPP

#include "fwd.hpp"

#include "bitwise.hpp"

namespace gem {
struct InterruptRegister {
   public:
    enum Registers {
        IE = 0xFFFF,
        IF = 0xFF0F,
    };
    enum class Bit : unsigned {
        VBlank = 0,
        STAT = 1,
        Timer = 2,
        Serial = 3,
        Joypad = 4,
    };
    enum class Handler : u16 {
        VBlank = 0x40,
        STAT = 0x48,
        Timer = 0x50,
        Serial = 0x58,
        Joypad = 0x60,
    };

    struct BitAndHandler {
        Bit bit;
        Handler handler;
    };
    static constexpr std::array<BitAndHandler, 5> bitAndHandlerPairs = {{
          {Bit::VBlank, Handler::VBlank},
          {Bit::STAT, Handler::STAT},
          {Bit::Timer, Handler::Timer},
          {Bit::Serial, Handler::Serial},
          {Bit::Joypad, Handler::Joypad},
    }};

    bool vblank() const noexcept { return test<idx(Bit::VBlank)>(); }
    bool stat() const noexcept { return test<idx(Bit::STAT)>(); }
    bool timer() const noexcept { return test<idx(Bit::Timer)>(); }
    bool serial() const noexcept { return test<idx(Bit::Serial)>(); }
    bool joypad() const noexcept { return test<idx(Bit::Joypad)>(); }

    void fireVBlank() noexcept { set<idx(Bit::VBlank)>(); }
    void fireStat() noexcept { set<idx(Bit::STAT)>(); }
    void fireTimer() noexcept { set<idx(Bit::Timer)>(); }
    void fireSerial() noexcept { set<idx(Bit::Serial)>(); }
    void fireJoypad() noexcept { set<idx(Bit::Joypad)>(); }

    u8 val = 0x00;

   private:
    template <unsigned Bit>
    bool test() const noexcept {
        return bitwise::test<Bit>(val);
    }
    template <unsigned Bit>
    void set() noexcept {
        bitwise::set<Bit>(val);
    }
};
}  // namespace gem

#endif
