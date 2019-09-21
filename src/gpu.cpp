#include "gpu.hpp"

#include "bitwise.hpp"
#include "fs.hpp"
#include "mem.hpp"
#include "screen.hpp"

#include <array>
#include <optional>

#include <fstream>
#include <iostream>

namespace gem {
using Color = GPU::Color;
constexpr Color operator""_argb(unsigned long long u) {
    const u8 a = u8((u & 0xFF000000) >> (6u * 4u));
    const u8 r = u8((u & 0x00FF0000) >> (4u * 4u));
    const u8 g = u8((u & 0x0000FF00) >> (2u * 4u));
    const u8 b = u8((u & 0x000000FF) >> (0u * 4u));
    return Color{a, r, g, b};
}

namespace {
// 0xAARRGGBB
namespace Colors {
constexpr auto Black = 0xFF000000_argb;
constexpr auto DarkGray = 0xFF555555_argb;
constexpr auto LightGray = 0xFFAAAAAA_argb;
constexpr auto White = 0xFFFFFFFF_argb;
};  // namespace Colors
}  // namespace

GPU::Tile::Tile(const u8* const data) {
    GEM_ASSERT(data != nullptr);
    std::size_t pixelsIdx = 0;
    for (std::size_t i = 0; i < 16; i += 2) {
        const u8 first = data[i];
        const u8 second = data[i + 1];
        for (u8 b = 8; b > 0; --b) {
            const u8 firstB = bitwise::test(first, b - 1);
            const u8 secondB = bitwise::test(second, b - 1);
            pixels[pixelsIdx] = ColorCode(u8(secondB << 1u) | firstB);
            ++pixelsIdx;
        }
    }
}

#ifndef NDEBUG
void GPU::Tile::dump() const {
    return;
    for (std::size_t i = 0; i < Width; ++i) {
        for (std::size_t j = 0; j < Height; ++j) {
            switch (this->pixels[i + 8 * j]) {
                case ColorCode::C00:
                    std::cout << ' ';
                    break;
                case ColorCode::C10:
                    std::cout << '#';
                    break;
                case ColorCode::C01:
                    std::cout << '*';
                    break;
                case ColorCode::C11:
                    std::cout << '&';
                    break;
            }
        }
        std::cout << '\n';
    }
}
#endif

GPU::CachedTile GPU::loadCachedTile(u16 address) const {
    const auto tile = CachedTile{vramPtr(address)};
#ifndef NDEBUG
    tile.dump();
#endif
    return tile;
}

void GPU::invalidateTileCacheForAddress(u16 address) {
    address /= Tile::MemSize;
    cachedTiles[address] = std::nullopt;
}

GPU::GPU(Screen& screen)
    : spriteData{Mem::makeBlock<0xFE00, 0xFE9F>()}
    , screen{screen}
    , vram{Mem::makeBlock<0x8000, 0x9FFF>()}
    , cachedTiles((TileSet0End - TileSet1Start) / Tile::MemSize, std::nullopt) {
}

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
void GPU::Mode_VBlank::step(GPU& gpu) {
    // VBlank takes the same amount of time as 10 scanlines including HBlank
    constexpr Ticks scanlineTime =
          (Mode_ScanlineOAM::Time + Mode_ScanlineVRAM::Time +
           Mode_HBlank::Time);
    static_assert(scanlineTime == Mode_VBlank::Time / 10);
    const auto lines = this->timer / scanlineTime;
    while (stepTimer < lines) {
        ++stepTimer;
        ++gpu.currentLine;
        GEM_ASSERT(gpu.currentLine <= 154);
    }
}
GPU::Mode GPU::Mode_VBlank::nextMode(GPU& gpu) {
    gpu.screen.get().vblank();
    GEM_ASSERT(gpu.currentLine == 154);
    gpu.currentLine = 0;
    return Mode_ScanlineOAM{};
}

void GPU::Mode_ScanlineOAM::step(GPU&) {}
GPU::Mode GPU::Mode_ScanlineOAM::nextMode(const GPU&) {
    return Mode_ScanlineVRAM{};
}
void GPU::Mode_ScanlineVRAM::step(GPU&) {}
GPU::Mode GPU::Mode_ScanlineVRAM::nextMode(GPU& gpu) {
    gpu.renderScanLine();
    return Mode_HBlank{};
}

namespace {
std::array<u8, 2> garbage{0x00, 0x00};
}
u8* GPU::registerPtr(const u16 address) {
    switch (address) {
        case Registers::SCROLLX:
            return &this->scrollX;
        case Registers::SCROLLY:
            return &this->scrollY;
        case Registers::LCDC:
            return &this->lcdc;
        case Registers::LY:
            return &this->currentLine;
        // TODO add more
        default:
            return garbage.data();
    }
}

namespace {

template <typename A>
decltype(auto) index(A&& a, std::size_t idx) {
#ifndef NDEBUG
    return a.at(idx);
#else
    return a[idx];
#endif
}

#ifndef NDEBUG
#define GEM_LOG_TILE_SET_MAP_CHANGES true
#endif

GPU::TileSet getTileSet(const u8 lcdc) {
    const GPU::TileSet tileSet =
          bitwise::test<4>(lcdc) ? GPU::TileSet::_0 : GPU::TileSet::_1;
#if GEM_LOG_TILE_SET_MAP_CHANGES
    static std::optional<GPU::TileSet> prevTileSet;
    if (std::exchange(prevTileSet, tileSet) != tileSet) {
        GEM_LOG("tile set changed to : "
                << (tileSet == GPU::TileSet::_0 ? "0" : "1"));
    }
#endif
    return tileSet;
}
GPU::TileMap getTileMap(const u8 lcdc) {
    const GPU::TileMap tileMap =
          bitwise::test<3>(lcdc) ? GPU::TileMap::_1 : GPU::TileMap::_0;
#if GEM_LOG_TILE_SET_MAP_CHANGES
    static std::optional<GPU::TileMap> prevTileMap;
    if (std::exchange(prevTileMap, tileMap) != tileMap) {
        GEM_LOG("tile map changed to : "
                << (tileMap == GPU::TileMap::_0 ? "0" : "1"));
    }
#endif
    return tileMap;
}

#undef GEM_LONG_TILE_SET_MAP_CHANGES

u16 getMapStart(const GPU::TileMap map) {
    const u16 start = [&] {
        switch (map) {
            case GPU::TileMap::_0:
                return GPU::TileMap0Start;
            case GPU::TileMap::_1:
                return GPU::TileMap1Start;
        }
        GEM_UNREACHABLE();
    }();
    GEM_ASSERT(start > GPU::VideoRAMStart);
    return start - GPU::VideoRAMStart;
}

u16 getTileNumber(const GPU::TileSet set, const u8 value) {
    switch (set) {
        case GPU::TileSet::_1:
            return value;
        case GPU::TileSet::_0: {
            i8 signedValue;
            std::memcpy(&signedValue, &value, sizeof signedValue);
            const u16 ret = u16(255 + signedValue);
            return ret;
        }
    }
    GEM_UNREACHABLE();
}

u16 getTileAddress(const GPU::TileSet set, const u8 value) {
    constexpr u16 tileSetsBeginning = GPU::TileSet1Start;
    const u16 tileNumber = getTileNumber(set, value);
    const u16 address = tileNumber * GPU::Tile::MemSize;
    return address + tileSetsBeginning - GPU::VideoRAMStart;
}

u16 getTileMapIndex(const u16 offsetX, const u16 offsetY, const u16 mapStart) {
    const u16 col = offsetX / GPU::Tile::Width;
    const u16 row = offsetY / GPU::Tile::Height;
    const u16 idx = col + row * 32;
    return mapStart + idx;
}

void dumpColor(const GPU::ColorCode c) {
    switch (c) {
        case GPU::ColorCode::C00:
            std::cout << ' ';
            break;
        case GPU::ColorCode::C10:
            std::cout << '#';
            break;
        case GPU::ColorCode::C01:
            std::cout << '*';
            break;
        case GPU::ColorCode::C11:
            std::cout << '&';
            break;
    }
}

Color pixelFromColorCode(const GPU::ColorCode cc) {
    switch (cc) {
        case GPU::ColorCode::C00:
            return Colors::White;
        case GPU::ColorCode::C10:
            return Colors::DarkGray;
        case GPU::ColorCode::C01:
            return Colors::LightGray;
        case GPU::ColorCode::C11:
            return Colors::Black;
    }
}

}  // namespace

void GPU::renderScanLine() {
    const TileMap tileMap = getTileMap(this->lcdc);
    const TileSet tileSet = getTileSet(this->lcdc);

    const u16 mapStart = getMapStart(tileMap);

    const u16 yOffset = this->scrollY + this->currentLine;

    std::array<u8, Screen::Width * 4> line;

    for (u16 i = 0; i < Screen::Width; ++i) {
        const u16 xOffset = i + this->scrollX;
        const u16 idx = getTileMapIndex(xOffset, yOffset, mapStart);
        const u8 tileValue = index(vram, idx);
        const u16 tileAddress = getTileAddress(tileSet, tileValue);

        std::optional<Tile>& tile =
              index(cachedTiles, tileAddress / Tile::MemSize);
        if (!tile) {
            tile = loadCachedTile(tileAddress);
        }
        const u16 pixelColumn = xOffset % GPU::Tile::Width;
        const u16 pixelRow = yOffset % GPU::Tile::Height;

        const ColorCode pixelCC =
              tile->pixels[pixelColumn + pixelRow * GPU::Tile::Width];
        const auto pixel = pixelFromColorCode(pixelCC);

        std::copy_n(pixel.begin(), pixel.size(), line.begin() + i * 4u);
    }
    this->screen.get().renderLine(line, yOffset);
}

void GPU::dumpTileMemory() {
    constexpr u16 totalTiles = 384;
    constexpr u16 tilesPerRow = 24;
    constexpr u16 tilesPerColumn = 16;
    static_assert(tilesPerRow * tilesPerColumn == totalTiles);
    constexpr std::size_t imageWidth = tilesPerRow * Tile::Width;
    constexpr std::size_t imageHeight = tilesPerColumn * Tile::Height;
    constexpr std::size_t bytesPerPixel = 3;
    std::string img = "P3\n" + std::to_string(imageWidth) + ' ' +
                      std::to_string(imageHeight) + "\n255\n";
    std::array<u8, imageWidth* imageHeight* bytesPerPixel> data = {};

    for (u16 tileNumber = 0; tileNumber < totalTiles; ++tileNumber) {
        const u16 tileAddress = tileNumber * Tile::MemSize;
        std::optional<Tile>& tile = index(cachedTiles, tileNumber);
        if (!tile) {
            tile = loadCachedTile(tileAddress);
        }

        for (unsigned r = 0; r < Tile::Height; ++r) {
            for (unsigned c = 0; c < Tile::Width; ++c) {
                const std::size_t column =
                      c + (tileNumber % tilesPerRow) * Tile::Width;
                const std::size_t row =
                      r + (tileNumber / tilesPerRow) * Tile::Height;
                const std::size_t index =
                      (column + row * imageWidth) * bytesPerPixel;

                const ColorCode pixelCC =
                      tile->pixels[c + r * GPU::Tile::Width];
                const auto pixel = pixelFromColorCode(pixelCC);

                std::copy_n(pixel.begin(), bytesPerPixel, data.data() + index);
            }
        }
    }

    for (std::size_t i = 0; i < data.size(); ++i) {
        img += std::to_string(data[i]);
        if (((i + 1) % (imageWidth * bytesPerPixel)) == 0) {
            img += '\n';
        } else {
            img += " ";
        }
    }

    std::ofstream out{fs::AbsolutePath{fs::RelativePath("tileData.ppm")}.path};
    GEM_ASSERT(out);
    out << img;
}

}  // namespace gem
