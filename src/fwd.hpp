#ifndef GEM_FWD_HPP
#define GEM_FWD_HPP

#include <cstdint>

namespace gem {

#ifndef NDEBUG
#define GEM_ENABLE_ASSERTS true
#endif

#if GEM_ENABLE_ASSERTS
#define GEM_ASSERT(...) assert(__VA_ARGS__)
#else
#define GEM_ASSERT(...) \
    do {                \
    } while (false)
#endif

#if GEM_ENABLE_ASSERTS
#define GEM_UNREACHABLE() GEM_ASSERT(false)
#else
// TODO platform detection
#define GEM_UNREACHABLE() __builtin_unreachable()
#endif

using u8 = unsigned char;
using u16 = std::uint16_t;

}  // namespace gem

#endif
