#include "mem.hpp"

#include "bootstrap.hpp"

#include <algorithm>

namespace gem {

Mem::Mem() : mem(::gem::loadBootstrapROM()) {
    mem.resize(std::numeric_limits<u16>::max(), 0xBE);
}

u8 Mem::read(u16 address) const {
    return this->mem[address];
}
void Mem::write(u16 address, u8 value) {
    this->mem[address] = value;
}

}  // namespace gem
