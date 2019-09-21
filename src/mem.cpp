#include "mem.hpp"

#include "bootstrap.hpp"

#include "gpu.hpp"
#include "io.hpp"

#include <array>

namespace {
constexpr std::array<gem::u8, 2> zeroData{0x00, 0x00};
std::array<gem::u8, 2> garbage{0x00, 0x00};
}  // namespace

namespace gem {

Mem::Mem(Block rom, GPU& gpu, IO& io)
    : zeroPage(makeBlock<0xFF80, 0xFFFF>())
    , ROM(makeBlock<0x00, 0x3FFF>(std::move(rom)))
    , bootstrap(::gem::loadBootstrapROM())
    , gpu{gpu}
    , io{io}
    , externalRam(makeBlock<0xA000, 0xBFFF>())
    , workingRam(makeBlock<0xC000, 0xDFFF>()) {}

u8 Mem::read(u16 address) const {
    return *ptr(address);
}

void Mem::write(u16 address, u8 value) {
    if (!io.consumeWrite(address, value)) {
        *mut_ptr(address) = value;
    }
}
void Mem::write(const u16 address, const u16 value) {
    std::memcpy(mut_ptr(address), &value, 2);
}

template <bool Write>
struct GetPtr {
    std::conditional_t<Write, u8*, const u8*> operator()(
          std::conditional_t<Write, Mem&, const Mem&> mem,
          const u16 address) const {
        switch (address & 0xF000) {
            case 0x0000: {
                if (false && !mem.bootstrap.empty() && address <= 0xFF) {
                    return mem.bootstrap.data() + address;
                }
                [[fallthrough]];
            }
            case 0x3000:
            case 0x2000:
            case 0x1000:
                return mem.ROM.data() + address;

            case 0x4000:
            case 0x5000:
            case 0x6000:
            case 0x7000:
                return mem.ROM.data() + address;

            case 0x8000:
            case 0x9000:
                if constexpr (Write) {
                    return mem.gpu.writableVramPtr(address - 0x8000);
                } else {
                    return mem.gpu.vramPtr(address - 0x8000);
                }

            case 0xA000:
            case 0xB000:
                return mem.externalRam.data() + (address - 0xA000);

            case 0xC000:
            case 0xD000:
                return mem.workingRam.data() + (address - 0xC000);

            // shadow working RAM
            case 0xE000:
                return mem.workingRam.data() + (address - 0xE000);

            case 0xF000: {
                switch (address & 0x0F00) {
                    // shadow working RAM
                    case 0x0100:
                    case 0x0200:
                    case 0x0300:
                    case 0x0400:
                    case 0x0500:
                    case 0x0600:
                    case 0x0700:
                    case 0x0800:
                    case 0x0A00:
                    case 0x0B00:
                    case 0x0C00:
                    case 0x0D00:
                        return mem.workingRam.data() + (address - 0xE000);

                    case 0x0E00:
                        if (address <= 0xFE9F) {
                            return mem.gpu.spriteData.data() +
                                   (address - 0xFE00);
                        }
                        if constexpr (Write) {
                            return ::garbage.data();
                        } else {
                            return ::zeroData.data();
                        }

                    case 0x0F00:
                        switch (address & 0x00F0) {
                            case 0x80:
                            case 0x90:
                            case 0xA0:
                            case 0xB0:
                            case 0xC0:
                            case 0xD0:
                            case 0xE0:
                            case 0xF0:
                                return mem.zeroPage.data() + (address - 0xFF80);

                            case 0x40:
                            case 0x50:
                            case 0x60:
                            case 0x70:
                                return mem.gpu.registerPtr(address);

                            default:
                                if constexpr (Write) {
                                    return mem.io.writableRegisterPtr(address);
                                } else {
                                    return mem.io.readOnlyRegisterPtr(address);
                                }
                        }
                }
            }
        }
        GEM_UNREACHABLE();
    }
};

const u8* Mem::ptr(u16 address) const {
    return GetPtr<false>{}(*this, address);
}

u8* Mem::mut_ptr(u16 address) {
    return GetPtr<true>{}(*this, address);
}

}  // namespace gem
