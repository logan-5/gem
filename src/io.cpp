#include "io.hpp"

#include "input.hpp"
#include "opcode.hpp"

#include <array>

namespace gem {

namespace {
constexpr std::array<u8, 2> zeros{0x00, 0x00};
constexpr std::array<u8, 2> ones{0xFF, 0xFF};
std::array<u8, 2> garbage{0xFF, 0xFF};
}  // namespace

const u8* IO::readOnlyRegisterPtr(const u16 address) const {
    switch (address) {
        case Registers::P1:
            return &this->p1;
        default:
            return blob.data() + (address - RegisterRange::Start);
    }
}

u8* IO::writableRegisterPtr(const u16 address) {
    GEM_ASSERT(!IO{*this}.consumeWrite(address, 0x00));
    return blob.data() + (address - RegisterRange::Start);
}

bool IO::consumeWrite(const u16 address, const u8 value) {
    switch (address) {
        case Registers::P1:
            this->p1 = value | 0b1100'0000;
            updateP1();
            return true;
        case Registers::SB:
            this->sb = value;
            return true;
        case Registers::SC:
            if (value == 0x81) {
                GEM_LOG_EXACTLY(this->sb);
            }
            return true;
    }
    return false;
}

void IO::updateP1() {
    switch (p1 & 0x30) {
        case 0x30:
            p1 |= 0x0F;
            break;
        case 0x20: {
            u8 nybble = 0xF0;
            if (Input::isButtonPressed(Input::Button::Down)) {
                nybble |= 0b1000;
            }
            if (Input::isButtonPressed(Input::Button::Up)) {
                nybble |= 0b0100;
            }
            if (Input::isButtonPressed(Input::Button::Left)) {
                nybble |= 0b0010;
            }
            if (Input::isButtonPressed(Input::Button::Right)) {
                nybble |= 0b0001;
            }
            p1 |= ~nybble;
        }
        case 0x10: {
            u8 nybble = 0xF0;
            if (Input::isButtonPressed(Input::Button::Start)) {
                nybble |= 0b1000;
            }
            if (Input::isButtonPressed(Input::Button::Select)) {
                nybble |= 0b0100;
            }
            if (Input::isButtonPressed(Input::Button::B)) {
                nybble |= 0b0010;
            }
            if (Input::isButtonPressed(Input::Button::A)) {
                nybble |= 0b0001;
            }
            p1 |= ~nybble;
        }
    }
}

void IO::update() {}

}  // namespace gem
