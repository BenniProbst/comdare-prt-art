#pragma once
// PoolDescriptor — Identitaet + Statistik eines Allocator-Pools
// REV 6 §5.17 — 4+2 Allocator-Pools (A/B/C/D + R + V-static/V-dynamic)

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace comdare::prt_art::allocator {

enum class PoolKind : std::uint8_t {
    A_TrieHuelle   = 0, // Trie-Huelle / Redirect-Knoten
    B_DensePages   = 1, // Dense-Byte / Dense-Multilevel Pages
    C_MultiLevel   = 2, // Multi-Level Multibyte-Pages (START)
    D_DecisionSpan = 3, // B+ / Decision-Span Pages
    R_Rest         = 4, // Restpool (Misc, kleine Allokationen)
    V_StaticValue  = 5, // Inline-Werte (kleine, statisch gegroessete Slots)
    V_DynamicValue = 6, // External + Chain-Werte (variable Groesse)
};

[[nodiscard]] constexpr std::string_view name(PoolKind k) noexcept {
    switch (k) {
        case PoolKind::A_TrieHuelle: return "A_TrieHuelle";
        case PoolKind::B_DensePages: return "B_DensePages";
        case PoolKind::C_MultiLevel: return "C_MultiLevel";
        case PoolKind::D_DecisionSpan: return "D_DecisionSpan";
        case PoolKind::R_Rest: return "R_Rest";
        case PoolKind::V_StaticValue: return "V_StaticValue";
        case PoolKind::V_DynamicValue: return "V_DynamicValue";
    }
    return "Unknown";
}

struct PoolStatistics {
    std::uint64_t allocations          = 0;
    std::uint64_t deallocations        = 0;
    std::uint64_t bytes_allocated      = 0;
    std::uint64_t bytes_deallocated    = 0;
    std::uint64_t peak_bytes_in_use    = 0;
    std::uint64_t current_bytes_in_use = 0;

    void on_alloc(std::size_t bytes) noexcept {
        ++allocations;
        bytes_allocated += bytes;
        current_bytes_in_use += bytes;
        if (current_bytes_in_use > peak_bytes_in_use) peak_bytes_in_use = current_bytes_in_use;
    }
    void on_dealloc(std::size_t bytes) noexcept {
        ++deallocations;
        bytes_deallocated += bytes;
        if (bytes <= current_bytes_in_use)
            current_bytes_in_use -= bytes;
        else
            current_bytes_in_use = 0;
    }
};

struct PoolDescriptor {
    PoolKind       kind;
    std::size_t    slot_size_bytes      = 0; // 0 = variable size
    std::size_t    capacity_slots       = 0;
    std::size_t    cache_line_alignment = 64;
    PoolStatistics stats{};

    [[nodiscard]] std::string_view name_view() const noexcept { return name(kind); }
};

} // namespace comdare::prt_art::allocator
