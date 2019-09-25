#include "cpu.hpp"

#ifndef NDEBUG
#define GEM_DEBUG_STACK false
#endif

#if GEM_DEBUG_STACK
#include <stack>
static std::stack<gem::u16> debugStack;
#endif

#if GEM_DEBUG_STACK
#define GEM_DEBUG_PUSH_STACK(...)       \
    do {                                \
        ::debugStack.push(__VA_ARGS__); \
    } while (false)
#else
#define GEM_DEBUG_PUSH_STACK(...) \
    do {                          \
    } while (false)
#endif
void gem::CPU::pushStack(gem::u16 val) {
    const u8 low8 = u8(val & 0x00FF);
    const u8 high8 = u8(val >> 8u);

    reg.decSP();
    bus.write(reg.getSP(), high8);
    reg.decSP();
    bus.write(reg.getSP(), low8);

    GEM_DEBUG_PUSH_STACK(val);
}
#undef GEM_DEBUG_PUSH_STACK

#if GEM_DEBUG_STACK
#define GEM_DEBUG_POP_STACK(...)                            \
    do {                                                    \
        \ GEM_ASSERT(!::debugStack.empty());                \
        \ GEM_ASSERT((__VA_ARGS__) == ::debugStack.top() && \
                     "stack corruption!");                  \
        \ ::debugStack.pop();                               \
    } while (false)
#else
#define GEM_DEBUG_POP_STACK(...) \
    do {                         \
    } while (false)
#endif
gem::u16 gem::CPU::popStack() {
    const u8 low8 = bus.read(reg.getSP());
    reg.incSP();
    const u8 high8 = bus.read(reg.getSP());
    reg.incSP();

    const u16 val = u16(high8 << 8u) | u16(low8);

    GEM_DEBUG_POP_STACK(val);
    return val;
}
#undef GEM_DEBUG_POP_STACK

namespace gem {

u8 CPU::getPendingInterrupts() const {
    return bus.interruptFlags.getMasked() & bus.enabledInterrupts.getMasked();
}

void CPU::processInterrupts() {
    if (((bus.enabledInterrupts.getMasked()) != 0) &&
        ((bus.interruptFlags.getMasked()) != 0)) {
        if (const u8 interruptsThatOccurred = getPendingInterrupts()) {
            halted = false;
            stopped = false;
            if (ime) {
                const auto& interrupts = Interrupt::bitAndHandlerPairs;
                for (auto& bhp : interrupts) {
                    if (bitwise::test(interruptsThatOccurred, idx(bhp.bit))) {
                        bitwise::reset(*bus.interruptFlags.valPtr(),
                                       idx(bhp.bit));
                        handleInterrupt(idx(bhp.handler));
                    }
                }
            }
        }
    }
}

void CPU::halt() {
    if (!ime && (getPendingInterrupts() != 0)) {
        haltBug = true;
    } else {
        halted = true;
    }
}

void CPU::handleInterrupt(const u16 destination) {
    ime = false;

    pushStack(reg.getPC());
    reg.setPC(destination);
}

void CPU::returnFromInterrupt() {
    ime = true;
    reg.setPC(popStack());
}

}  // namespace gem
