#pragma once
// Vector<u16,u16> internal search — Density >75% (REV 6 §5.17)
//
// Sortiertes Vector von (uint16_t-Diskriminator, child_index)-Paaren fuer
// hochdichte Verzweigung. Verwendet wird wenn Density sowohl von Array<65535>
// als auch von Vector<u8,u8> ueberschritten wird.

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

namespace comdare::prt_art::internal_search {

class VectorU16U16 {
public:
    static constexpr double kDensityMinPercent = 75.0;
    static constexpr std::size_t kCapacity = 65535;   // u16-Diskriminatoren

    struct Entry {
        std::uint16_t discriminator;
        std::uint64_t child_index;
    };

    void insert(std::uint16_t discriminator, std::uint64_t child_index) {
        auto it = std::lower_bound(entries_.begin(), entries_.end(), discriminator,
            [](Entry const& e, std::uint16_t d) { return e.discriminator < d; });
        if (it != entries_.end() && it->discriminator == discriminator) {
            it->child_index = child_index;
        } else {
            entries_.insert(it, {discriminator, child_index});
        }
    }

    [[nodiscard]] std::optional<std::uint64_t> lookup(std::uint16_t discriminator) const noexcept {
        auto it = std::lower_bound(entries_.begin(), entries_.end(), discriminator,
            [](Entry const& e, std::uint16_t d) { return e.discriminator < d; });
        if (it != entries_.end() && it->discriminator == discriminator)
            return it->child_index;
        return std::nullopt;
    }

    void erase(std::uint16_t discriminator) noexcept {
        auto it = std::lower_bound(entries_.begin(), entries_.end(), discriminator,
            [](Entry const& e, std::uint16_t d) { return e.discriminator < d; });
        if (it != entries_.end() && it->discriminator == discriminator)
            entries_.erase(it);
    }

    [[nodiscard]] std::size_t occupied_count() const noexcept { return entries_.size(); }

    [[nodiscard]] double density_percent() const noexcept {
        return (static_cast<double>(entries_.size()) / static_cast<double>(kCapacity)) * 100.0;
    }

    [[nodiscard]] std::vector<Entry> const& entries() const noexcept { return entries_; }

private:
    std::vector<Entry> entries_{};
};

}  // namespace comdare::prt_art::internal_search
