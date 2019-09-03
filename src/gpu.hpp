#ifndef GEM_GPU_HPP
#define GEM_GPU_HPP

#include "fwd.hpp"
#include "mem.hpp"

namespace gem {
struct GPU {
    enum Registers : u16 {
        SCROLLX = 0xFF43,
        SCROLLY = 0xFF42,
        LCDC = 0xFF40,
        WNDPOSX = 0xFF4B,
        WNDPOSY = 0xFF4A,
    };

    enum : u16 {
        VideoRAMStart = 0x8000,
        VideoRAMEnd = 0xA000,
    };

    explicit GPU()
        : vram{Mem::makeBlock<0x8000, 0x9FFF>()}
        , spriteData{Mem::makeBlock<0xFE00, 0xFE9F>()} {}

    Mem::Block vram;
    Mem::Block spriteData;
};
}  // namespace gem

#endif
