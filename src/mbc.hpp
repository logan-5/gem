#ifndef GEM_MBC_HPP
#define GEM_MBC_HPP

#include "fwd.hpp"

#include <variant>

namespace gem {

namespace MBCMode {
struct None {};
struct MBC1 {};
}  // namespace MBCMode

struct MBC {
    enum : u16 {
        Selector = 0x0147,
    };

    using Mode = std::variant<MBCMode::None>;

    explicit MBC(std::vector<u8> rom);

    bool consumeWrite(const u16 address, const u8 val);

    const u8* ptr(const u16 address) const;
    u8* ptr(const u16 address);

   private:
    std::vector<u8> rom;
    std::vector<u8> externalRam;
    Mode mode;

    bool ramEnabled() const;

    template <bool>
    struct GetPtr;
};

}  // namespace gem

#endif
