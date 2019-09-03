#include "rom.hpp"
#include "fs.hpp"

#include <fstream>
#include <optional>
#include <vector>

std::optional<gem::Mem::Block> gem::ROM::load(
      const gem::fs::AbsolutePath& path) {
    std::ifstream fstr{path.path.c_str(), std::ios::binary};
    if (!fstr.is_open()) {
        return std::nullopt;
    }

    return gem::Mem::Block(std::istreambuf_iterator<char>{fstr},
                           std::istreambuf_iterator<char>{});
}
