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

    enum RegisterRange {
        Start = 0xFF00,
        End = 0xFF40,
    };

    const u8* readOnlyRegisterPtr(const u16 address) const;
    u8* writableRegisterPtr(const u16 address);

    bool consumeWrite(const u16 address, const u8 value);

    void update();

   private:
    u8 p1{0xFF};
    u8 sb;

    std::array<u8, RegisterRange::End - RegisterRange::Start> blob;
};

}  // namespace gem

#endif
