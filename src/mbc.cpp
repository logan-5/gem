#include "mbc.hpp"
#include "mem.hpp"

#include <array>

namespace gem {

namespace {

template <typename... Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};
template <typename... Fs>
overloaded(Fs...)->overloaded<Fs...>;

constexpr std::array<u8, 2> ones{{0xFF, 0xFF}};
std::array<u8, 2> garbage{{0x00, 0x00}};

MBC::Mode getMode(u8 mbcSelector) {
    switch (mbcSelector) {
        case 0:
            return MBCMode::None{};
    }
    throw std::runtime_error{"unsupported MBC type :("};
}
}  // namespace

MBC::MBC(std::vector<u8> rom)
    : rom{Mem::makeBlock<0x0000, 0x3FFF>(std::move(rom))}
    , externalRam{Mem::makeBlock<0xA000, 0xBFFF>()}
    , mode{getMode(this->rom[Selector])} {}

template <bool Write>
struct MBC::GetPtr {
    using Ptr = std::conditional_t<Write, u8*, const u8*>;
    using MBCRef = std::conditional_t<Write, MBC&, const MBC&>;
    Ptr operator()(MBCRef mbc, u16 address) const {
        if (address <= 0x3FFF) {
            return mbc.rom.data() + address;
        }
        if (0x4000 <= address && address <= 0x7FFF) {
            return get4000_7FFF(mbc, address);
        }
        if (mbc.ramEnabled()) {
            return mbc.externalRam.data() + (address - 0xA000);
        } else {
            if constexpr (Write) {
                return garbage.data();
            } else {
                return ones.data();
            }
        }
    }

    Ptr get4000_7FFF(MBCRef mbc, const u16 address) const {
        using namespace MBCMode;
        return std::visit(
              overloaded{[&](None) { return mbc.rom.data() + address; }},
              mbc.mode);
    }
};

const u8* MBC::ptr(const u16 address) const {
    return GetPtr<false>{}(*this, address);
}

u8* MBC::ptr(const u16 address) {
    GEM_ASSERT(!consumeWrite(address, 0x0));
    return GetPtr<true>{}(*this, address);
}

bool MBC::consumeWrite(const u16 address, const u8 value) {
    return false;
}

bool MBC::ramEnabled() const {
    return false;
}

}  // namespace gem
