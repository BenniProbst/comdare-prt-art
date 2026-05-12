#pragma once
// MultiLevelLayout — Cache-Tier-aware Slot-Anordnung (L1/L2/L3-Tiers)
// REV 6 §5.17 — kombiniert TLB-Offset-Adressierung mit Cache-Hierarchy-Awareness
//
// Drei Tiers:
//   Tier 0 (L1)  — heisse Slots (≤32 KiB Working-Set)
//   Tier 1 (L2)  — warme Slots (≤1 MiB)
//   Tier 2 (L3)  — kalte Slots (≤32 MiB)
//
// Der Tier wird anhand des virtuellen Offsets gewaehlt, sodass haeufig zugegriffene
// Bereiche im niedrigeren Cache-Tier landen.

#include <prt_art/memory_layout/cache_line_aligned_layout.hpp>
#include <prt_art/memory_layout/virtual_offset_address.hpp>

#include <cstddef>
#include <cstdint>

namespace comdare::prt_art::memory_layout {

enum class CacheTier : std::uint8_t {
    L1Hot   = 0,
    L2Warm  = 1,
    L3Cold  = 2,
    Memory  = 3,
};

struct TierBudget {
    std::size_t l1_bytes = 32   * 1024;        // 32 KiB
    std::size_t l2_bytes = 1024 * 1024;        // 1 MiB
    std::size_t l3_bytes = 32   * 1024 * 1024; // 32 MiB
};

class MultiLevelLayout {
public:
    explicit MultiLevelLayout(TierBudget budget = {}) : budget_(budget) {}

    [[nodiscard]] CacheTier tier_for_offset(std::uint64_t byte_offset) const noexcept {
        if (byte_offset < budget_.l1_bytes) return CacheTier::L1Hot;
        if (byte_offset < budget_.l1_bytes + budget_.l2_bytes) return CacheTier::L2Warm;
        if (byte_offset < budget_.l1_bytes + budget_.l2_bytes + budget_.l3_bytes)
            return CacheTier::L3Cold;
        return CacheTier::Memory;
    }

    [[nodiscard]] std::uint64_t resolve_address(std::span<std::byte const> key,
                                                std::size_t slot_size_bytes) const noexcept {
        std::uint64_t pos = VirtualOffsetAddress::compute(key);
        std::size_t aligned_slot = CacheLineAlignedLayout::aligned_offset(slot_size_bytes);
        return pos * static_cast<std::uint64_t>(aligned_slot);
    }

    [[nodiscard]] CacheTier tier_for_key(std::span<std::byte const> key,
                                         std::size_t slot_size_bytes) const noexcept {
        return tier_for_offset(resolve_address(key, slot_size_bytes));
    }

    [[nodiscard]] TierBudget const& budget() const noexcept { return budget_; }

    void set_budget(TierBudget b) noexcept { budget_ = b; }

private:
    TierBudget budget_;
};

}  // namespace comdare::prt_art::memory_layout
