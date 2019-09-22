#include "fs.hpp"
#include "gem_config.hpp"

#include <fstream>

namespace gem {
namespace util {
template <typename String>
bool starts_with(const String& str, const typename String::value_type val) {
    return !str.empty() && str.front() == val;
}
template <typename String>
bool ends_with(const String& str, const typename String::value_type val) {
    return !str.empty() && str.back() == val;
}
inline bool starts_with(const char* str, const char val) {
    return str[0] == val;
}
inline bool ends_with(const char* str, const char val) {
    const auto len = std::strlen(str);
    return len > 0 && str[len - 1] == val;
}
}  // namespace util

namespace {
std::string makeAbsolute(std::string_view rel) {
    // TODO portability
    const auto projectPath = fs::projectPath();
    const bool needsSlash =
          !(util::ends_with(projectPath, '/') || util::starts_with(rel, '/'));
    std::string path = projectPath;
    if (needsSlash) {
        path += '/';
    }
    path += rel;
    return path;
}
}  // namespace

}  // namespace gem

gem::fs::AbsolutePath::AbsolutePath(RelativePathView rv)
    : path{makeAbsolute(rv.path)} {}

void gem::fs::write(std::string_view data, const AbsolutePath& outPath) {
    std::ofstream out{outPath.path};
    GEM_ASSERT(out);
    out << data;
}
