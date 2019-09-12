#ifndef GEM_GPU_HPP
#define GEM_GPU_HPP

#include "fwd.hpp"
#include "mem.hpp"

#include <array>
#include <optional>
#include <variant>
#include <vector>

namespace gem {
struct GPU {
    enum Registers : u16 {
        SCROLLX = 0xFF43,
        SCROLLY = 0xFF42,
        LCDC = 0xFF40,
        WNDPOSX = 0xFF4B,
        WNDPOSY = 0xFF4A,
        LY = 0xFF44,
    };

    enum : u16 {
        VideoRAMStart = 0x8000,
        VideoRAMEnd = 0xA000,

        TileSet1Start = 0x8000,
        TileSet1End = 0x9000,
        TileSet0Start = 0x8800,
        TileSet0End = 0x9800,
        TileMap0Start = 0x9800,
        TileMap0End = 0x9C00,
        TileMap1Start = 0x9C00,
        TileMap1End = 0xA000,
    };

    enum class TileSet : u8 {
        _0,
        _1,
    };
    enum class TileMap : u8 {
        _0,
        _1,
    };

    explicit GPU();

    const u8* vramPtr(const u16 address) const { return vram.data() + address; }
    u8* writableVramPtr(const u16 address) {
        if (address < TileSet0End - VideoRAMStart) {
            invalidateTileCacheForAddress(address);
        }
        return vram.data() + address;
    }
    const u8* registerPtr(const u16 address) const {
        return const_cast<GPU*>(this)->registerPtr(address);
    }
    u8* registerPtr(const u16 address);

    Mem::Block spriteData;

    void step(DeltaTicks deltaTicks);

    struct Color {
        u8 a, r, g, b;
    };

    enum ColorCode : u8 {
        C00 = 0b00,
        C01 = 0b01,
        C10 = 0b10,
        C11 = 0b11,
    };
    struct Tile {
        explicit Tile(const u8* const data);
        static constexpr u8 Width = 8, Height = 8;
        static constexpr u8 MemSize = 2 * Height;
        std::array<ColorCode, Width * Height> pixels;

#ifndef NDEBUG
        void dump() const;
#endif
    };

   private:
    Mem::Block vram;

    struct Screen {
        static constexpr u8 Width = 160, Height = 144;

        Color& operator()(std::size_t w, std::size_t h) { return data[w][h]; }
        const Color& operator()(std::size_t w, std::size_t h) const {
            return data[w][h];
        }

        std::array<std::array<Color, Height>, Width> data;
    };

    struct Mode;
    struct Mode_Base {
        Mode_Base() {}
        Ticks timer = 0;
    };
    struct Mode_ScanlineOAM final : Mode_Base {
        enum {
            Number = 2,
        };
        enum : Ticks {
            Time = 80,
        };
        static void step(GPU& gpu);
        static Mode nextMode(const GPU& gpu);
    };
    struct Mode_ScanlineVRAM final : Mode_Base {
        enum {
            Number = 3,
        };
        enum : Ticks {
            Time = 172,
        };
        static void step(GPU& gpu);
        static Mode nextMode(GPU& gpu);
    };
    struct Mode_HBlank final : Mode_Base {
        enum {
            Number = 0,
        };
        enum : Ticks {
            Time = 204,
        };
        static void step(const GPU&) {}
        static Mode nextMode(GPU& gpu);
    };
    struct Mode_VBlank final : Mode_Base {
        enum {
            Number = 1,
        };
        enum : Ticks {
            Time = 4560,
        };
        void step(GPU&);
        static Mode nextMode(GPU& gpu);

       private:
        u8 stepTimer = 0;
    };

    struct Mode
        : std::variant<Mode_HBlank,
                       Mode_VBlank,
                       Mode_ScanlineOAM,
                       Mode_ScanlineVRAM> {
        using variant::variant;
    };

    Mode mode;
    u8 currentLine = 0;
    u8 lcdc = 0;
    u8 scrollX = 0;
    u8 scrollY = 0;

    using CachedTile = Tile;
    CachedTile loadCachedTile(u16 address) const;
    void invalidateTileCacheForAddress(u16 address);
    std::vector<std::optional<CachedTile>> cachedTiles;

    void renderScanLine();
};
}  // namespace gem

#endif
