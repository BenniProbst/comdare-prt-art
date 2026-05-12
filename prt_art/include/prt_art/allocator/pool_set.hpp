#pragma once
// PoolSet — alle 7 Allocator-Pools verwaltet (4 Page-Pools + R + V-static + V-dynamic)
// REV 6 §5.17 — kombiniert PoolDescriptors zu einem konsistenten Pool-System

#include <prt_art/allocator/pool_descriptor.hpp>

#include <array>
#include <cstdint>
#include <string_view>

namespace comdare::prt_art::allocator {

class PoolSet {
public:
    static constexpr std::size_t kPoolCount = 7;

    PoolSet() {
        descriptors_[index(PoolKind::A_TrieHuelle)].kind     = PoolKind::A_TrieHuelle;
        descriptors_[index(PoolKind::B_DensePages)].kind     = PoolKind::B_DensePages;
        descriptors_[index(PoolKind::C_MultiLevel)].kind     = PoolKind::C_MultiLevel;
        descriptors_[index(PoolKind::D_DecisionSpan)].kind   = PoolKind::D_DecisionSpan;
        descriptors_[index(PoolKind::R_Rest)].kind           = PoolKind::R_Rest;
        descriptors_[index(PoolKind::V_StaticValue)].kind    = PoolKind::V_StaticValue;
        descriptors_[index(PoolKind::V_DynamicValue)].kind   = PoolKind::V_DynamicValue;
    }

    [[nodiscard]] PoolDescriptor& get(PoolKind k) noexcept {
        return descriptors_[index(k)];
    }
    [[nodiscard]] PoolDescriptor const& get(PoolKind k) const noexcept {
        return descriptors_[index(k)];
    }

    void configure(PoolKind k, std::size_t slot_size, std::size_t capacity) noexcept {
        auto& d = get(k);
        d.slot_size_bytes = slot_size;
        d.capacity_slots  = capacity;
    }

    void note_alloc(PoolKind k, std::size_t bytes) noexcept {
        get(k).stats.on_alloc(bytes);
    }
    void note_dealloc(PoolKind k, std::size_t bytes) noexcept {
        get(k).stats.on_dealloc(bytes);
    }

    [[nodiscard]] std::uint64_t total_allocated_bytes() const noexcept {
        std::uint64_t sum = 0;
        for (auto const& d : descriptors_) sum += d.stats.bytes_allocated;
        return sum;
    }

    [[nodiscard]] std::uint64_t total_in_use_bytes() const noexcept {
        std::uint64_t sum = 0;
        for (auto const& d : descriptors_) sum += d.stats.current_bytes_in_use;
        return sum;
    }

    [[nodiscard]] static constexpr std::size_t index(PoolKind k) noexcept {
        return static_cast<std::size_t>(k);
    }

    [[nodiscard]] std::array<PoolDescriptor, kPoolCount> const& all() const noexcept {
        return descriptors_;
    }

private:
    std::array<PoolDescriptor, kPoolCount> descriptors_{};
};

}  // namespace comdare::prt_art::allocator
