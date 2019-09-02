#ifndef GEM_ROM_HPP
#define GEM_ROM_HPP

#include "fs.hpp"
#include "fwd.hpp"

#include <vector>

namespace gem {
namespace ROM {
std::optional<std::vector<u8>> load(const fs::AbsolutePath& path);
}
}  // namespace gem

#endif
