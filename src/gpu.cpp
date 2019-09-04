#include "gpu.hpp"

#include "bitwise.hpp"
#include "mem.hpp"

#include <array>
#include <optional>

namespace gem {
using Color = GPU::Color;
constexpr Color operator""_argb(unsigned long long u) {
    const u8 a = u8(u & 0xFF000000 >> 6u);
    const u8 r = u8(u & 0x00FF0000 >> 4u);
    const u8 g = u8(u & 0x0000FF00 >> 2u);
    const u8 b = u8(u & 0x000000FF >> 0u);
    return Color{a, r, g, b};
}

namespace {

enum ColorCode : u8 {
    C00 = 0b00,
    C01 = 0b01,
    C10 = 0b10,
    C11 = 0b11,
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-const-variable"
// 0xAARRGGBB
namespace Colors {
constexpr auto Black = 0xFF000000_argb;
constexpr auto DarkGray = 0xFF555555_argb;
constexpr auto LightGray = 0xFFAAAAAA_argb;
constexpr auto White = 0xFFFFFFFF_argb;
};  // namespace Colors
#pragma clang diagnostic pop

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

void GPU::step(DeltaTicks deltaTicks) {
    if (auto nextMode = std::visit(
              [&](auto& mode) -> std::optional<Mode> {
                  using ModeType = std::decay_t<decltype(mode)>;
                  mode.timer += deltaTicks;
                  mode.step(*this);
                  if (mode.timer >= ModeType::Time) {
                      return mode.nextMode(*this);
                  }
                  return std::nullopt;
              },
              mode)) {
        mode = *std::move(nextMode);
    }
}

GPU::Mode GPU::Mode_HBlank::nextMode(GPU& gpu) {
    ++gpu.currentLine;
    if (gpu.currentLine == Screen::Height) {
        return Mode_VBlank{};
    }
    return Mode_ScanlineOAM{};
}
void GPU::Mode_VBlank::step(const GPU&) {
    // VBlank takes the same amount of time as 10 scanlines including HBlank
    static_assert((Mode_ScanlineOAM::Time + Mode_ScanlineVRAM::Time +
                   Mode_HBlank::Time) == Mode_VBlank::Time / 10);
    while (this->timer > Mode_HBlank::Time) {
        this->timer -= Mode_HBlank::Time;
        ++this->lines;
    }
}
GPU::Mode GPU::Mode_VBlank::nextMode(const GPU&) {
    return Mode_ScanlineOAM{};
}

void GPU::Mode_ScanlineOAM::step(GPU&) {}
GPU::Mode GPU::Mode_ScanlineOAM::nextMode(const GPU&) {
    return Mode_ScanlineVRAM{};
}
void GPU::Mode_ScanlineVRAM::step(GPU&) {}
GPU::Mode GPU::Mode_ScanlineVRAM::nextMode(GPU&) {
    // render scan
    return Mode_HBlank{};
}

}  // namespace gem
