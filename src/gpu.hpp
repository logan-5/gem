#ifndef GEM_GPU_HPP
#define GEM_GPU_HPP

#include "fwd.hpp"
#include "mem.hpp"

#include <array>
#include <variant>

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

    void step(DeltaTicks deltaTicks);

    struct Color {
        u8 a, r, g, b;
    };

   public:
    struct Screen {
        static constexpr u8 Width = 160, Height = 144;

        Color& operator()(std::size_t w, std::size_t h) { return data[w][h]; }
        const Color& operator()(std::size_t w, std::size_t h) const {
            return data[w][h];
        }

        std::array<std::array<Color, Height>, Width> data;
    } screen;

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
        void step(const GPU&);
        static Mode nextMode(const GPU& gpu);

       private:
        u8 lines = 0;
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
};
}  // namespace gem

#endif
