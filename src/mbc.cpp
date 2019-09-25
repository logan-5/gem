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

MBC::Mode getMode(const u8 mbcSelector) {
    switch (mbcSelector) {
        case 0x0:
            return MBCMode::None{};
        case 0x1:
        case 0x2:
        case 0x3:
            return MBCMode::MBC1{};
        case 0x12:
        case 0x13:
            return MBCMode::MBC3{};
    }
    throw std::runtime_error{"unsupported MBC type :("};
}

constexpr usize eightK = 0x2000;

usize ramSize(const MBC::Mode& m) {
    using namespace MBCMode;
    return std::visit(overloaded{
                            [](None) { return eightK; },
                            [](const MBC1&) { return eightK * 4; },
                            [](const MBC3&) { return eightK * 8; },
                      },
                      m);
}

std::vector<u8> makeRam(const MBC::Mode& mode) {
    return std::vector<u8>(ramSize(mode), 0x00);
}

}  // namespace

MBC::MBC(std::vector<u8> rom)
    : rom{Mem::makeBlock<0x0000, 0x3FFF>(std::move(rom))}
    , mode{getMode(this->rom[Selector])}
    , externalRam{makeRam(this->mode)} {}

template <bool Write>
struct MBC::GetPtr {
    using Ptr = std::conditional_t<Write, u8*, const u8*>;
    template <typename T>
    using Ref = std::conditional_t<Write, T&, const T&>;
    using MBCRef = Ref<MBC>;
    Ptr operator()(MBCRef mbc, u16 address) const {
        if (address <= 0x3FFF) {
            return mbc.rom.data() + address;
        }
        if (0x4000 <= address && address <= 0x7FFF) {
            return get4000_7FFF(mbc, address);
        }
        if (mbc.ramEnabled()) {
            return getExternalRam(mbc, address);
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
              overloaded{
                    [&](None) { return mbc.rom.data() + address; },
                    [&](const MBC1& m) {
                        const usize romBank =
                              m.romBankLower5 |
                              ((m.quux << 5) &
                               u8((m.quuxMode == MBC1::QuuxMode::RAM) - 1));
                        return mbc.rom.data() + (address - 0x4000) +
                               romBank * 0x4000;
                    },
                    [&](const MBC3& m) {
                        const usize romBank = m.romBankLower7;
                        return mbc.rom.data() + (address - 0x4000) +
                               romBank * 0x4000;
                    }},
              mbc.mode);
    }

    Ptr getExternalRam(MBCRef mbc, const u16 address) const {
        GEM_ASSERT(mbc.ramEnabled());
        using namespace MBCMode;
        return std::visit(
              overloaded{
                    [&](None) {
                        return mbc.externalRam.data() + address - 0xA000;
                    },
                    [&](const MBC1& m) {
                        const usize ramBank =
                              0x2000 *
                              (m.quux &
                               u8((m.quuxMode == MBC1::QuuxMode::ROM) - 1));
                        return mbc.externalRam.data() +
                               (ramBank + address - 0xA000);
                    },
                    [&](Ref<MBC3>& m) {
                        if (m.ramOrRTC >= 0x08) {
                            return m.rtcRegisters.data() + (m.ramOrRTC - 0x08);
                        }
                        const usize ramBank = 0x2000 * m.ramOrRTC;
                        return mbc.externalRam.data() +
                               (ramBank + address - 0xA000);
                    }},
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
    using namespace MBCMode;
    return std::visit(
          overloaded{[](None) { return false; },
                     [&](MBC1& m) {
                         if (address <= 0x1FFF) {
                             m.ramEnabled = (value & 0x0F) == 0x0A;
                             return true;
                         }
                         if (0x2000 <= address && address <= 0x3FFF) {
                             m.romBankLower5 = value & 0x1F + (value == 0x00);
                             return true;
                         }
                         if (0x4000 <= address && address <= 0x5FFF) {
                             m.quux = value & 0b11;
                             return true;
                         }
                         if (0x6000 <= address && address <= 0x7FFF) {
                             m.quuxMode = (value & 0x1) ? MBC1::QuuxMode::RAM
                                                        : MBC1::QuuxMode::ROM;
                             return true;
                         }
                         return false;
                     },
                     [&](MBC3& m) {
                         if (address <= 0x1FFF) {
                             m.ramRTCEnabled = (value & 0x0F) == 0x0A;
                             return true;
                         }
                         if (0x2000 <= address && address <= 0x3FFF) {
                             m.romBankLower7 = (value & 0x7F) + (value == 0x00);
                             return true;
                         }
                         if (0x4000 <= address && address <= 0x5FFF) {
                             m.ramOrRTC = value;
                             return true;
                         }
                         if (0x6000 <= address && address <= 0x7FFF) {
                             // TODO latch clock data
                             return true;
                         }
                         return false;
                     }},
          mode);
}

bool MBC::ramEnabled() const {
    using namespace MBCMode;
    return std::visit(overloaded{
                            [](None) { return true; },
                            [](const MBC1& m) { return m.ramEnabled; },
                            [](const MBC3& m) { return m.ramRTCEnabled; },
                      },
                      mode);
}

}  // namespace gem
