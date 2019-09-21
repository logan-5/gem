#include "io.hpp"

#include <array>

namespace gem {

namespace {
constexpr std::array<u8, 2> zeros{0x00, 0x00};
constexpr std::array<u8, 2> ones{0xFF, 0xFF};
std::array<u8, 2> garbage{0x00, 0x00};
}  // namespace

const u8* IO::readOnlyRegisterPtr(const u16 address) const {
    switch (address) {
        case Registers::P1:
            return &this->p1;
        default:
            return zeros.data();
    }
}

u8* IO::writableRegisterPtr(const u16 address) {
    GEM_ASSERT(!consumeWrite(address, 0x00));
    return garbage.data();
}

bool IO::consumeWrite(const u16 address, const u8 value) {
    switch (address) {
        case Registers::P1:
            this->p1 = value | 0x0F;
            return true;
        case Registers::SB:
            this->sb = value;
            return true;
        case Registers::SC:
            if (value == 0x81) {
                GEM_LOG_EXACTLY(this->sb);
            }
    }
    return false;
}

void IO::update() {}

}  // namespace gem
