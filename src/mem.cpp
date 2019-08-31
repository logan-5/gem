#include "mem.hpp"

namespace gem {

Mem::Mem() : mem(std::numeric_limits<u16>::max(), 0xBE) {}

u8 Mem::read(u16 address) {
    return this->mem[address];
}
void Mem::write(u16 address, u8 value) {
    this->mem[address] = value;
}

}  // namespace gem
