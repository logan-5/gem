#ifndef GEM_FWD_HPP
#define GEM_FWD_HPP

#include <array>
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

#include <iostream>
#define GEM_LOG_EXACTLY(...)      \
    do {                          \
        std::cerr << __VA_ARGS__; \
    } while (false)
#define GEM_LOG(...) GEM_LOG_EXACTLY(__VA_ARGS__ << '\n')

#ifndef NDEBUG
#define GEM_DEBUG_LOGGING true
#endif

#if GEM_DEBUG_LOGGING
#include <iostream>
#define GEM_DEBUG_LOG(...) GEM_LOG(__VA_ARGS__)
#else
#define GEM_DEBUG_LOG(...) \
    do {                   \
    } while (false)
#endif

namespace gem {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using usize = std::size_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;

using Ticks = unsigned long long;
using DeltaTicks = unsigned long long;

template <std::size_t Size>
struct TinyString {
    constexpr TinyString() { buf[Size] = '\0'; }
    static constexpr std::size_t size() { return Size; }
    constexpr char& operator[](std::size_t s) { return buf[s]; }
    constexpr const char& operator[](std::size_t s) const { return buf[s]; }
    constexpr const char* data() const { return buf.data(); }
    constexpr char* data() { return buf.data(); }

   private:
    std::array<char, Size + 1> buf;
};

template <std::size_t Size>
std::ostream& operator<<(std::ostream& ostr, const TinyString<Size>& s) {
    return ostr << s.data();
}

constexpr gem::TinyString<4> hexString(const u16 val) {
    constexpr auto lookup = "0123456789ABCDEF";
    gem::TinyString<4> ret;
    ret[0] = lookup[(val >> 12) & 0xF];
    ret[1] = lookup[(val >> 8) & 0xF];
    ret[2] = lookup[(val >> 4) & 0xF];
    ret[3] = lookup[(val >> 0) & 0xF];
    return ret;
}

}  // namespace gem

#endif
