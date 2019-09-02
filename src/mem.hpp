#ifndef GEM_MEM_HPP
#define GEM_MEM_HPP

#include "fwd.hpp"

#include <vector>

namespace gem {

struct Mem {
    explicit Mem(std::vector<u8> rom);

    u8 read(u16 address) const;
    void write(u16 address, u8 value);
    void write(u16 address, u16 value);

    const u8* ptr(u16 address) const {
        GEM_ASSERT(address < mem.size());
        return mem.data() + address;
    }

   private:
    u8* mut_ptr(u16 address) {
        GEM_ASSERT(address < mem.size());
        return mem.data() + address;
    }
    std::vector<u8> mem;
    std::vector<u8> romFirst256;
};

}  // namespace gem

#endif
