#ifndef GEM_ROM_HPP
#define GEM_ROM_HPP

#include "fs.hpp"
#include "fwd.hpp"
#include "mem.hpp"

#include <vector>

namespace gem {
namespace ROM {
std::optional<Mem::Block> load(const fs::AbsolutePath& path);
}
}  // namespace gem

#endif
