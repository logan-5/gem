#ifndef GEM_MEM_HPP
#define GEM_MEM_HPP

#include "fwd.hpp"

#include <vector>

namespace gem {

struct Mem {
    explicit Mem();

    u8 read(u16 address);
    void write(u16 address, u8 value);

   private:
    std::vector<u8> mem;
};

}  // namespace gem

#endif
