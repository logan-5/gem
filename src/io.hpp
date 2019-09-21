#ifndef GEM_IO_HPP
#define GEM_IO_HPP

#include "fwd.hpp"

namespace gem {

struct IO {
    enum Registers : u16 {
        P1 = 0xFF00,
        SB = 0xFF01,
        SC = 0xFF02,
    };

    const u8* readOnlyRegisterPtr(const u16 address) const;
    u8* writableRegisterPtr(const u16 address);

    bool consumeWrite(const u16 address, const u8 value);

    void update();

   private:
    u8 p1{0x3F};
    u8 sb;
};

}  // namespace gem

#endif
