#pragma once
// Array<256> internal search — Density bis 25% (REV 6 §5.17)
//
// Direkt-adressiertes Array von 256 child_index-Eintraegen, jeweils via
// std::uint8_t-Diskriminator indexiert. Sehr schneller Zugriff (1 Cache-Line),
// aber Speicher-Overhead bei niedriger Density.

#include <array>
#include <cstdint>
#include <optional>

namespace comdare::prt_art::internal_search {

class Array256 {
public:
    static constexpr std::size_t kCapacity = 256;
    static constexpr double      kDensityMaxPercent = 25.0;

    Array256() {
        for (auto& slot : slots_) slot = kEmpty;
    }

    void insert(std::uint8_t discriminator, std::uint64_t child_index) noexcept {
        slots_[discriminator] = child_index;
    }

    [[nodiscard]] std::optional<std::uint64_t> lookup(std::uint8_t discriminator) const noexcept {
        std::uint64_t v = slots_[discriminator];
        if (v == kEmpty) return std::nullopt;
        return v;
    }

    void erase(std::uint8_t discriminator) noexcept {
        slots_[discriminator] = kEmpty;
    }

    [[nodiscard]] std::size_t occupied_count() const noexcept {
        std::size_t n = 0;
        for (auto v : slots_) if (v != kEmpty) ++n;
        return n;
    }

    [[nodiscard]] double density_percent() const noexcept {
        return (static_cast<double>(occupied_count()) / static_cast<double>(kCapacity)) * 100.0;
    }

private:
    static constexpr std::uint64_t kEmpty = static_cast<std::uint64_t>(-1);
    std::array<std::uint64_t, kCapacity> slots_{};
};

}  // namespace comdare::prt_art::internal_search
