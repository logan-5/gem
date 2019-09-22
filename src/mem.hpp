#ifndef GEM_MEM_HPP
#define GEM_MEM_HPP

#include "fwd.hpp"

#include "interrupt.hpp"

#include <vector>

namespace gem {

struct GPU;
struct IO;

struct Mem {
    using Block = std::vector<u8>;

    explicit Mem(Block rom, GPU& gpu, IO& io);

    u8 read(u16 address) const;
    void write(u16 address, u8 value);
    void write(u16 address, u16 value);

    const u8* ptr(u16 address) const;

    template <std::size_t Start, std::size_t EndInclusive>
    static Block makeBlock(Block block = {}) {
        static_assert(Start <= EndInclusive);
        if (block.size() < EndInclusive - Start + 1) {
            block.resize(EndInclusive - Start + 1, 0x00);
        }
        return block;
    }

    InterruptRegister enabledInterrupts, interruptFlags;

   private:
    template <bool>
    friend struct GetPtr;
    u8* mut_ptr(u16 address);

    Block zeroPage;
    Block ROM;
    Block bootstrap;
    GPU& gpu;
    IO& io;
    Block externalRam;
    Block workingRam;
};

}  // namespace gem

#endif
