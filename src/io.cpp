#include "io.hpp"

#include "bitwise.hpp"
#include "input.hpp"
#include "mem.hpp"
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
        case Registers::DIV:
            return &this->div;
        case Registers::TIMA:
            return &this->timer;
        case Registers::TMA:
            return &this->tma;
        case Registers::TAC:
            return &this->tac;
        default:
            return blob.data() + (address - RegisterRange::Start);
    }
}

u8* IO::writableRegisterPtr(const u16 address) {
    GEM_ASSERT(!IO{*this}.consumeWrite(address, 0x00));
    switch (address) {
        case Registers::TIMA:
            return &this->timer;
        case Registers::TMA:
            return &this->tma;
        case Registers::TAC:
            return &this->tac;
        default:
            return blob.data() + (address - RegisterRange::Start);
    }
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
        case Registers::DIV:
            this->div = 0x00;
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
            break;
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
            break;
        }
    }
}

namespace {
u16 getTimerThreshold(const u8 tac) {
    switch (tac & 0b11) {
        case 0b00:
            return 64;
        case 0b01:
            return 1;
        case 0b10:
            return 4;
        case 0b11:
            return 16;
    }
    GEM_UNREACHABLE();
}
}  // namespace

void IO::update(Ticks ticks) {
    timerSubCounter += ticks;
    while (timerSubCounter > 4) {
        ++timerCounter;
        timerSubCounter -= 4;

        divCounter = (divCounter + 1) % 16;
        if (divCounter == 0) {
            ++div;
        }
    }
    const auto timerThreshold = getTimerThreshold(tac);
    while (timerCounter > timerThreshold) {
        incTimer();
        timerCounter -= timerThreshold;
    }
}

void IO::incTimer() {
    if (bitwise::test<2>(tac)) {
        const u16 newTimer = static_cast<u16>(timer) + 1;

        const bool fireInterrupt = newTimer > 0xFF;
        if (fireInterrupt) {
            timer = tma;
            GEM_ASSERT(mem != nullptr);
            mem->interruptFlags.fireTimer();
        } else {
            timer = u8(newTimer);
        }
    }
}

}  // namespace gem
