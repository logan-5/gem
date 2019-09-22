#ifndef GEM_FS_HPP
#define GEM_FS_HPP

#include "fwd.hpp"

#include <optional>
#include <string>
#include <string_view>

// no <filesystem> on my macbook at the moment :(

namespace gem {
namespace fs {

struct RelativePath {
    explicit RelativePath(std::string p) : path{std::move(p)} {}
    std::string path;
};

struct RelativePathView {
    explicit RelativePathView(std::string_view p) : path{std::move(p)} {}
    RelativePathView(const RelativePath& p) : path{p.path} {}
    std::string_view path;
};

struct AbsolutePath {
    explicit AbsolutePath(std::string p) : path{std::move(p)} {}
    explicit AbsolutePath(RelativePathView rv);
    std::string path;
};

struct AbsolutePathView {
    explicit AbsolutePathView(std::string_view p) : path{std::move(p)} {}
    AbsolutePathView(const AbsolutePath& p) : path{p.path} {}
    std::string_view path;
};

void write(std::string_view data, const AbsolutePath& path);

}  // namespace fs
}  // namespace gem

#endif
