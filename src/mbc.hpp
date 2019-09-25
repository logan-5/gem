#ifndef GEM_MBC_HPP
#define GEM_MBC_HPP

#include "fwd.hpp"

#include <variant>

namespace gem {

namespace MBCMode {
struct None {};
struct MBC1 {
    bool ramEnabled = true;
    u8 romBankLower5 = 0x01;
    u8 quux = 0x00;
    enum class QuuxMode {
        ROM,
        RAM,
    } quuxMode = QuuxMode::ROM;
};
}  // namespace MBCMode

struct MBC {
    enum : u16 {
        Selector = 0x0147,
    };

    using Mode = std::variant<MBCMode::None, MBCMode::MBC1>;

    explicit MBC(std::vector<u8> rom);

    bool consumeWrite(const u16 address, const u8 val);

    const u8* ptr(const u16 address) const;
    u8* ptr(const u16 address);

   private:
    std::vector<u8> rom;

    Mode mode;
    std::vector<u8> externalRam;

    bool ramEnabled() const;

    template <bool>
    struct GetPtr;
};

}  // namespace gem

#endif
