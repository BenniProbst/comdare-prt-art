#pragma once
// PrtArtSearchEngine — komponierte ISearchEngine-Implementierung der PRT-ART
// REV 6 Final
//
// Komponiert alle PRT-ART-Bausteine zu einer ISearchEngine, die dann gegen den
// Stand-der-Technik (CacheEngine) experimentell evaluiert wird.
//
// Stub-Implementierung: lookup/insert/erase/range_scan-Skeletten. Volle Implementation
// kommt in Phase 7 (Permutations-Builds + Experiment-Loop).

#include <prt_art/allocator/pool_set.hpp>
#include <prt_art/concurrency/olc_with_reserved_blocks.hpp>
#include <prt_art/identity/prt_art_identity.hpp>
#include <prt_art/internal_search/array_256.hpp>
#include <prt_art/internal_search/vector_u16_u16.hpp>
#include <prt_art/measurement/density_tracker.hpp>
#include <prt_art/measurement/hypothesis_metrics.hpp>
#include <prt_art/memory_layout/multi_level_layout.hpp>
#include <prt_art/nodes/bplus_node.hpp>
#include <prt_art/nodes/redirect_node.hpp>
#include <prt_art/prefetch/path_oriented_prefetch.hpp>
#include <prt_art/value_handle/value_handle.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace comdare::prt_art::identity {

template <typename Key, typename Value, std::size_t InlineCapacity = 8>
class PrtArtSearchEngine {
public:
    using value_handle_t = ::comdare::prt_art::value_handle::ValueHandle<InlineCapacity>;

    PrtArtSearchEngine() {
        // Konfiguriere die 4+2 Pools mit Standard-Slot-Groessen
        pools_.configure(::comdare::prt_art::allocator::PoolKind::A_TrieHuelle,    64,   1024);
        pools_.configure(::comdare::prt_art::allocator::PoolKind::B_DensePages,   256,   1024);
        pools_.configure(::comdare::prt_art::allocator::PoolKind::C_MultiLevel,  1024,    512);
        pools_.configure(::comdare::prt_art::allocator::PoolKind::D_DecisionSpan, 4096,   256);
        pools_.configure(::comdare::prt_art::allocator::PoolKind::R_Rest,           0,      0);
        pools_.configure(::comdare::prt_art::allocator::PoolKind::V_StaticValue,   16,   8192);
        pools_.configure(::comdare::prt_art::allocator::PoolKind::V_DynamicValue,   0,   2048);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // ISearchEngine Public-API (std::map-aehnlich) — Stub
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::optional<Value> lookup(Key const&) const {
        // Stub — volle Implementation in Phase 7
        return std::nullopt;
    }

    bool insert(Key const&, Value const&) {
        // Stub
        ++inserts_;
        return true;
    }

    bool erase(Key const&) {
        // Stub
        return false;
    }

    [[nodiscard]] std::size_t size()  const noexcept { return size_; }
    [[nodiscard]] bool        empty() const noexcept { return size_ == 0; }

    // ─────────────────────────────────────────────────────────────────────────
    // Identitaets-API — fuer CacheEngineBuilder
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] static ::comdare::cache_engine::PermutationFlags identity_flags() noexcept {
        return prt_art_permutation_flags();
    }

    [[nodiscard]] static std::string identifier() {
        return prt_art_identifier();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Komponenten-Accessors (fuer Inspection / Tests)
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] auto& pools()              noexcept { return pools_; }
    [[nodiscard]] auto& concurrency()        noexcept { return concurrency_; }
    [[nodiscard]] auto& memory_layout()      noexcept { return memory_layout_; }
    [[nodiscard]] auto& path_prefetch()      noexcept { return path_prefetch_; }
    [[nodiscard]] auto& density_tracker()    noexcept { return density_tracker_; }
    [[nodiscard]] auto& hypothesis_metrics() noexcept { return hypothesis_metrics_; }

    [[nodiscard]] std::uint64_t total_inserts() const noexcept { return inserts_; }

private:
    ::comdare::prt_art::allocator::PoolSet               pools_{};
    ::comdare::prt_art::concurrency::OlcWithReservedValueBlocks concurrency_{};
    ::comdare::prt_art::memory_layout::MultiLevelLayout  memory_layout_{};
    ::comdare::prt_art::prefetch::PathOrientedPrefetch   path_prefetch_{};
    ::comdare::prt_art::measurement::DensityTracker      density_tracker_{};
    ::comdare::prt_art::measurement::PrtArtHypothesisMetrics hypothesis_metrics_{};

    std::size_t   size_    = 0;
    std::uint64_t inserts_ = 0;
};

}  // namespace comdare::prt_art::identity
