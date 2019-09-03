#include "gpu.hpp"

#include "bitwise.hpp"
#include "mem.hpp"

#include <array>

namespace gem {

namespace {

enum ColorCode : u8 {
    C00 = 0b00,
    C01 = 0b01,
    C10 = 0b10,
    C11 = 0b11,
};

// 0xAARRGGBB
enum Color : u32 {
    Black = 0xFF000000,
    DarkGray = 0xFF555555,
    LightGray = 0xFFAAAAAA,
    White = 0xFFFFFFFF,
};

struct Tile {
    explicit Tile(const u8* const data) {
        std::size_t pixelsIdx = 0;
        for (std::size_t i = 0; i < 16; i += 2) {
            const u8 first = data[i];
            const u8 second = data[i + 1];
            for (u8 b = 8; b > 0; --b) {
                const u8 firstB = bitwise::test(first, b - 1);
                const u8 secondB = bitwise::test(second, b - 1);
                pixels[pixelsIdx] = ColorCode(u8(secondB << 1u) | firstB);
            }
        }
    }
    // static constexpr u8 Width = 8, Height = 8;
    std::array<ColorCode, 128> pixels;
};
}  // namespace

}  // namespace gem
