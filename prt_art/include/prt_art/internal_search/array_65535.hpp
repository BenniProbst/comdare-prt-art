#pragma once
// Array<65535> internal search — Density 25-50% (REV 6 §5.17)
//
// Direkt-adressiertes Array von 65535 child_index-Eintraegen, indexiert via
// std::uint16_t-Diskriminator. Hoehere Verzweigungs-Tiefe als Array<256>.

#include <cstdint>
#include <optional>
#include <vector>

namespace comdare::prt_art::internal_search {

class Array65535 {
public:
    static constexpr std::size_t kCapacity =
        65536; // M-PA-02-Fix: uint16-Diskriminator hat 65536 Werte (0..65535); insert(65535) war OOB bei 65535.
    static constexpr double kDensityMinPercent = 25.0;
    static constexpr double kDensityMaxPercent = 50.0;

    Array65535() : slots_(kCapacity, kEmpty) {}

    void insert(std::uint16_t discriminator, std::uint64_t child_index) noexcept {
        slots_[discriminator] = child_index;
    }

    [[nodiscard]] std::optional<std::uint64_t> lookup(std::uint16_t discriminator) const noexcept {
        std::uint64_t v = slots_[discriminator];
        if (v == kEmpty) return std::nullopt;
        return v;
    }

    void erase(std::uint16_t discriminator) noexcept { slots_[discriminator] = kEmpty; }

    [[nodiscard]] std::size_t occupied_count() const noexcept {
        std::size_t n = 0;
        for (auto v : slots_)
            if (v != kEmpty) ++n;
        return n;
    }

    [[nodiscard]] double density_percent() const noexcept {
        return (static_cast<double>(occupied_count()) / static_cast<double>(kCapacity)) * 100.0;
    }

private:
    static constexpr std::uint64_t kEmpty = static_cast<std::uint64_t>(-1);
    std::vector<std::uint64_t>     slots_;
};

} // namespace comdare::prt_art::internal_search
