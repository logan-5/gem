#ifndef GEM_FWD_HPP
#define GEM_FWD_HPP

#include <cstdint>

#ifndef NDEBUG
#define GEM_ENABLE_ASSERTS true
#endif

#if GEM_ENABLE_ASSERTS
#include <cassert>
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

#ifndef NDEBUG
// #define GEM_DEBUG_LOGGING true
#endif

#if GEM_DEBUG_LOGGING
#include <iostream>
#define GEM_DEBUG_LOG(...)                \
    do {                                  \
        std::cerr << __VA_ARGS__ << '\n'; \
    } while (false)
#else
#define GEM_DEBUG_LOG(...) \
    do {                   \
    } while (false)
#endif

namespace gem {

using u8 = unsigned char;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

using i8 = signed char;
using i16 = std::int16_t;
using i32 = std::int32_t;

using Ticks = unsigned long long;
using DeltaTicks = unsigned long long;

}  // namespace gem

#endif
