#ifndef GEM_MEM_HPP
#define GEM_MEM_HPP

#include "fwd.hpp"

#include <vector>

namespace gem {

struct Mem {
    explicit Mem();

    u8 read(u16 address) const;
    void write(u16 address, u8 value);

    const u8* ptr(u16 address) const {
        GEM_ASSERT(address < mem.size());
        return mem.data() + address;
    }

   private:
    std::vector<u8> mem;
};

}  // namespace gem

#endif
