#ifndef GEM_GPU_HPP
#define GEM_GPU_HPP

#include "fwd.hpp"
#include "mem.hpp"

#include <array>
#include <optional>
#include <variant>
#include <vector>

namespace gem {

struct Screen;

struct GPU {
    enum Registers : u16 {
        SCROLLX = 0xFF43,
        SCROLLY = 0xFF42,
        LCDC = 0xFF40,
        STAT = 0xFF41,
        WNDPOSX = 0xFF4B,
        WNDPOSY = 0xFF4A,
        LY = 0xFF44,
        LYC = 0xFF45,
        DMA = 0xFF46,

        // palettes
        BGP = 0xFF47,
        OBP0 = 0xFF48,
        OBP1 = 0xFF49,
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

        WindowMap0Start = 0x9800,
        WindowMap0End = 0x9BFF,
        WindowMap1Start = 0x9C00,
        WindowMap1End = 0x9FFF,

        SpriteTableStart = 0x8000,
        SpriteTableEnd = 0x9000,
    };

    enum class TileSet : u8 { _0, _1 };
    enum class TileMap : u8 { _0, _1 };
    enum class WindowTileMap : u8 { _0, _1 };
    enum class Palette : u8 { _0, _1 };
    enum class Priority : u8 { Front, Behind };

    struct OAM {
        u8 screenPosXPlus8;
        u8 screenPosYPlus16;
        u8 tileNumber;
        Palette palette;
        Priority priority;
        bool xFlip;
        bool yFlip;
    };
    struct SpriteData {
        enum : u16 {
            TotalSprites = 40,
            OAMBlockSize = 4,
            Start = 0xFE00,
            End = 0xFEA0,
        };
        static_assert(TotalSprites * OAMBlockSize == (End - Start));
        std::array<u8, End - Start> block = {};
    };

    struct Color {
       public:
        constexpr Color(u8 a, u8 r, u8 g, u8 b) : storage{{r, g, b, a}} {}

        constexpr auto begin() noexcept { return storage.begin(); }
        constexpr auto begin() const noexcept { return storage.begin(); }
        constexpr auto cbegin() const noexcept { return storage.begin(); }

        static constexpr auto size() noexcept {
            return std::tuple_size_v<decltype(storage)>;
        }

        bool operator==(Color other) const noexcept {
            return storage == other.storage;
        }
        bool operator!=(Color other) const noexcept {
            return !(*this == other);
        }
        bool operator==(const u8* buf) const noexcept {
            return std::memcmp(storage.data(), buf, storage.size()) == 0;
        }

       private:
        std::array<u8, 4> storage;
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
    };

    explicit GPU(Screen& screen);

    void setMem(Mem* const mem) { this->mem = mem; }

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
    const u8* spriteDataPtr(const u16 address) const {
        return spriteData.block.data() + address;
    }
    u8* writableSpriteDataPtr(const u16 address) {
        invalidateOAMCacheForAddress(address);
        return vram.data() + address;
    }
    bool consumeWrite(const u16 address, const u8 value);

    void step(DeltaTicks deltaTicks);

    void updateSTAT();

    void dmaTransfer();

    bool lcdEnabled() const;
    bool spritesEnabled() const;
    bool windowEnabled() const;

    std::vector<usize> findSpritesIntersectingCurrentLine();

   private:
    std::reference_wrapper<Screen> screen;
    Mem* mem = nullptr;

    Mem::Block vram;
    SpriteData spriteData;

    struct Mode;
    struct Mode_Base {
        Mode_Base() {}
        Ticks timer = 0;
    };
    struct Mode_ScanlineOAM final : Mode_Base {
        enum : u8 {
            Number = 0b10,
        };
        enum : Ticks {
            Time = 80,
        };
        static void step(GPU& gpu);
        static Mode nextMode(const GPU& gpu);
    };
    struct Mode_ScanlineVRAM final : Mode_Base {
        enum : u8 {
            Number = 0b11,
        };
        enum : Ticks {
            Time = 172,
        };
        static void step(GPU& gpu);
        static Mode nextMode(GPU& gpu);
    };
    struct Mode_HBlank final : Mode_Base {
        enum : u8 {
            Number = 0b00,
        };
        enum : Ticks {
            Time = 204,
        };
        static void step(const GPU&) {}
        static Mode nextMode(GPU& gpu);
    };
    struct Mode_VBlank final : Mode_Base {
        enum : u8 {
            Number = 0b01,
        };
        enum : Ticks {
            Time = 4560,
        };
        void step(GPU&);
        static Mode nextMode(GPU& gpu);

        explicit Mode_VBlank(GPU& gpu);

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
    u8 currentLine = u8(-1);
    u8 lcdc = 0;
    u8 scrollX = 0;
    u8 scrollY = 0;
    u8 stat = 0;
    u8 lyc = 0;
    bool statSignal = false;
    u8 dma = 0;
    u8 bgp = 0;
    u8 obp0 = 0;
    u8 obp1 = 0;

    u8 wx = 0;
    u8 wy = 0;

    // set once from WY at the beginning of each screen draw
    u8 currentWindowY = 0;
    u8 windowLine = 0;

    using CachedTile = Tile;
    CachedTile loadCachedTile(u16 address) const;
    void invalidateTileCacheForAddress(u16 address);
    std::vector<std::optional<CachedTile>> cachedTiles;

    OAM loadCachedOAM(u16 address) const;
    void invalidateOAMCacheForAddress(u16 address);
    void invalidateAllOAMCache();
    std::vector<std::optional<OAM>> cachedSprites;

    void renderScanLine();

    void dumpTileMemory();
    void dumpBackgroundMap(TileMap map);
};
}  // namespace gem

#endif
