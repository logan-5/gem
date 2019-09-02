#include "mem.hpp"

#include "bootstrap.hpp"

#include <algorithm>

namespace gem {

namespace {

std::vector<u8> makeMem(std::vector<u8> rom) {
    rom.resize(std::numeric_limits<u16>::max(), 0x00);
    return rom;
}

std::vector<u8> first256(const std::vector<u8>& ofVec) {
    GEM_ASSERT(ofVec.size() >= 256);
    return std::vector<u8>(ofVec.begin(), ofVec.begin() + 256);
}
}  // namespace

Mem::Mem(std::vector<u8> rom)
    : mem(makeMem(std::move(rom))), romFirst256(first256(mem)) {
    auto bootstrap = ::gem::loadBootstrapROM();
    GEM_ASSERT(bootstrap.size() == romFirst256.size());
    std::copy(bootstrap.begin(), bootstrap.end(), mem.begin());
}

u8 Mem::read(u16 address) const {
    GEM_ASSERT(address < mem.size());
    return this->mem[address];
}
void Mem::write(u16 address, u8 value) {
    GEM_ASSERT(address < mem.size());
    this->mem[address] = value;
}
void Mem::write(const u16 address, const u16 value) {
    std::memcpy(mut_ptr(address), &value, 2);
}

}  // namespace gem
