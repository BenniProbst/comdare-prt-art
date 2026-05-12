#pragma once
// VirtualOffsetAddress — TLB-inspirierte Mehrebenen-Adressierungs-Funktion
// REV 6 §5.17 — position = sum(k[i] * 256^(k_count-1-i))
//
// Konvertiert eine Byte-Sequenz (Schluessel-Praefix mit k_count Bytes) in eine
// virtuelle Position innerhalb eines hierarchisch organisierten Slot-Arrays.
//
// Beispiel (k_count=3, k=[A,B,C]):  pos = A*65536 + B*256 + C
// Erlaubt direkten Slot-Zugriff in O(1) bei bekanntem k_count.
//
// k_count > 8 wuerde uint64_t ueberlaufen — kapazitaetsgrenze.

#include <cstddef>
#include <cstdint>
#include <span>

namespace comdare::prt_art::memory_layout {

class VirtualOffsetAddress {
public:
    static constexpr std::size_t kMaxKeyCount = 8;
    static constexpr std::uint64_t kBase      = 256;

    [[nodiscard]] static constexpr std::uint64_t compute(std::span<std::byte const> key) noexcept {
        std::size_t n = key.size() <= kMaxKeyCount ? key.size() : kMaxKeyCount;
        std::uint64_t position = 0;
        for (std::size_t i = 0; i < n; ++i) {
            position = position * kBase + static_cast<std::uint64_t>(key[i]);
        }
        return position;
    }

    [[nodiscard]] static constexpr std::uint64_t capacity_for(std::size_t k_count) noexcept {
        if (k_count == 0) return 0;
        if (k_count >= kMaxKeyCount) return UINT64_MAX;
        std::uint64_t cap = 1;
        for (std::size_t i = 0; i < k_count; ++i) cap *= kBase;
        return cap;
    }

    // Inverse: Rekonstruiert den k_count-Byte-Praefix aus einer Position.
    // Schreibt n_bytes in out_bytes (highest-significant-byte first).
    static constexpr void decompose(std::uint64_t position,
                                    std::size_t k_count,
                                    std::byte* out_bytes) noexcept {
        if (k_count == 0 || k_count > kMaxKeyCount) return;
        for (std::size_t i = k_count; i-- > 0;) {
            out_bytes[i] = static_cast<std::byte>(position & 0xFFu);
            position >>= 8;
        }
    }
};

}  // namespace comdare::prt_art::memory_layout
