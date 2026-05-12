#pragma once
// CacheLineAlignedLayout — 64-Byte-aligned Slot-Layout
// REV 6 §5.17 — fuer konsequent cache-line-konforme Adressierung
//
// Liefert utility-Funktionen, um Slot-Offsets auf 64-Byte-Grenzen zu runden,
// Padding zu berechnen und Slot-Spans aus einem rohen Byte-Buffer zu liefern.

#include <cstddef>
#include <cstdint>
#include <span>

namespace comdare::prt_art::memory_layout {

class CacheLineAlignedLayout {
public:
    static constexpr std::size_t kCacheLineBytes = 64;

    [[nodiscard]] static constexpr std::size_t aligned_offset(std::size_t raw_offset) noexcept {
        std::size_t rem = raw_offset % kCacheLineBytes;
        return rem == 0 ? raw_offset : raw_offset + (kCacheLineBytes - rem);
    }

    [[nodiscard]] static constexpr std::size_t padding_to_next_line(std::size_t raw_offset) noexcept {
        std::size_t rem = raw_offset % kCacheLineBytes;
        return rem == 0 ? 0 : (kCacheLineBytes - rem);
    }

    [[nodiscard]] static constexpr std::size_t lines_needed(std::size_t bytes) noexcept {
        return (bytes + kCacheLineBytes - 1) / kCacheLineBytes;
    }

    [[nodiscard]] static constexpr bool is_aligned(std::uintptr_t address) noexcept {
        return (address % kCacheLineBytes) == 0;
    }

    // Wrapper: liefert den Slot bei index, wo jeder Slot kSlotBytes gross ist
    // und alle Slots cache-line-aligned beginnen.
    template <std::size_t SlotBytes>
    [[nodiscard]] static std::span<std::byte> slot_view(std::span<std::byte> buffer,
                                                         std::size_t slot_index) noexcept {
        static_assert(SlotBytes > 0, "SlotBytes muss > 0 sein");
        std::size_t aligned_slot = aligned_offset(SlotBytes);    // pad pro Slot auf next 64
        std::size_t offset = slot_index * aligned_slot;
        if (offset + SlotBytes > buffer.size()) return {};
        return buffer.subspan(offset, SlotBytes);
    }
};

}  // namespace comdare::prt_art::memory_layout
