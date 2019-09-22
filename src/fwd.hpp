#ifndef GEM_FWD_HPP
#define GEM_FWD_HPP

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

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

#ifndef NDEBUG
#include <csignal>
#define GEM_BREAKPOINT() std::raise(SIGTRAP)
#else
#define GEM_BREAKPOINT() \
    do {                 \
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

    template <std::size_t BufSize,
              typename = std::enable_if_t<BufSize <= Size + 1>>
    TinyString(const char (&buffer)[BufSize]) {
        std::memcpy(this->buf.data(), buffer, BufSize);
    }

    template <std::size_t OtherSize,
              typename = std::enable_if_t<(OtherSize < Size)>>
    TinyString& operator=(const TinyString<OtherSize>& other) {
        std::memcpy(this->buf.data(), other.data(), OtherSize);
        this->buf[OtherSize] = '\0';
        return *this;
    }
    template <std::size_t OtherSize,
              typename = std::enable_if_t<(OtherSize < Size)>>
    TinyString(const TinyString<OtherSize>& other) {
        *this = other;
    }

    TinyString(const TinyString&) = default;
    TinyString& operator=(const TinyString&) = default;

    template <std::size_t OtherSize>
    constexpr TinyString<Size + OtherSize> operator+(
          const TinyString<OtherSize>& other) const {
        TinyString<Size + OtherSize> ret;
        std::memcpy(ret.data(), this->data(), Size);
        std::memcpy(ret.data() + Size, other.data(), OtherSize + 1);
        return ret;
    }

    static constexpr std::size_t size() { return Size; }
    constexpr char& operator[](std::size_t s) { return buf[s]; }
    constexpr const char& operator[](std::size_t s) const { return buf[s]; }
    constexpr const char* data() const { return buf.data(); }
    constexpr char* data() { return buf.data(); }

   private:
    std::array<char, Size + 1> buf;
};

template <std::size_t Size>
constexpr TinyString<Size - 1> tiny_str(const char (&str)[Size]) noexcept {
    return TinyString<Size - 1>{str};
}

template <std::size_t Size>
std::ostream& operator<<(std::ostream& ostr, const TinyString<Size>& s) {
    return ostr << s.data();
}

namespace detail {
constexpr inline auto hexLookup = "0123456789ABCDEF";
}

constexpr inline gem::TinyString<2> hexString(const u8 val) {
    gem::TinyString<2> ret;
    ret[0] = detail::hexLookup[(val >> 4) & 0xF];
    ret[1] = detail::hexLookup[(val >> 0) & 0xF];
    return ret;
}

constexpr inline gem::TinyString<4> hexString(const u16 val) {
    gem::TinyString<4> ret;
    ret[0] = detail::hexLookup[(val >> 12) & 0xF];
    ret[1] = detail::hexLookup[(val >> 8) & 0xF];
    ret[2] = detail::hexLookup[(val >> 4) & 0xF];
    ret[3] = detail::hexLookup[(val >> 0) & 0xF];
    return ret;
}

template <typename T>
class FlatSet {
   public:
    FlatSet() = default;
    FlatSet(std::vector<T> ts) : storage{std::move(ts)} {
        std::sort(storage.begin(), storage.end());
    }
    bool contains(const T& t) const {
        return std::binary_search(storage.begin(), storage.end(), t);
    }

   private:
    std::vector<T> storage;
};

}  // namespace gem

#endif
