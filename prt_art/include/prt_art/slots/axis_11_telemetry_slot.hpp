#pragma once
// V41.F.6.1 Phase B — prt-art axis_11 Telemetry Slot (Plugin-Controller Slot-Füllung).
//
// Vierte Slot-Füllung (nach axis_07/01/14): prt-art liefert einen CE-konformen TELEMETRY-Wrapper
// (PrtArtPerNodeCounter) — das F15-ANTI-PATTERN-Validierungsinstrument (Mail Kuehn 2026-05-08):
// Counter in ALLEN Knoten (auch Inner-Nodes) → Cache-Line-Ping-Pong. Wird in der Diplomarbeit als
// Vergleichsbasis gegen die CE-Hauptvariante LeafOnlyCounter gemessen (H/F15-Hypothese:
// LeafOnly signifikant schneller unter Multi-Reader-Workload).
//
// Via pruefling_merge ZUR COMPILE-TIME in den CE-Permutationsraum gemergt — so kann der Mess-Treiber
// die beiden Telemetrie-Strategien im selben Permutations-Sweep gegenüberstellen.

#include <topics/telemetry/axis_11_telemetry/axis_11_telemetry_strategy_base.hpp>
#include <topics/telemetry/axis_11_telemetry/concepts/axis_11_telemetry_concept.hpp>
#include <topics/telemetry/axis_11_telemetry/concepts/axis_11_telemetry_cache_engine_permutation_concept.hpp>
#include <topics/telemetry/concepts/topic_telemetry_concept.hpp>
#include <anatomy/pruefling_merge.hpp>   // PrueflingSlot-Pattern + AnatomyGenus

#include <prt_art/telemetry/leaf_only_counter.hpp>  // prt-art PerNodeCounter (Anti-Pattern)

#include <boost/mp11.hpp>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace comdare::prt_art::slots::axis_11 {

namespace ce11 = ::comdare::cache_engine::telemetry::axis_11_telemetry;
namespace pf   = ::comdare::cache_engine::anatomy::pruefling;
namespace mp   = ::boost::mp11;

/// prt-art-eigene Subaxis-Markierung (per-node scope = Anti-Pattern).
struct per_node_scope_tag {};

/// PrtArtPerNodeCounter — CE-konformer axis_11-TELEMETRY-Wrapper über prt-arts PerNodeCounter.
/// F15-Anti-Pattern (Counter in ALLEN Knoten) — Vergleichsinstrument gegen LeafOnlyCounter.
/// Erfüllt TelemetryStrategy + CacheEnginePermutationStrategy → via pruefling_merge in den
/// CE-Permutationsraum einsetzbar (Stufe 3 full-join für den LeafOnly-vs-PerNode-Vergleich).
class PrtArtPerNodeCounter : public ce11::TelemetryStrategyBase<PrtArtPerNodeCounter> {
public:
    using topic_tag = ::comdare::cache_engine::telemetry::concepts::TelemetryTopicTag;
    using axis_tag  = per_node_scope_tag;
    using family_id = std::integral_constant<int, 14>;  // prt-art Telemetry-Range (distinkt zu CE)

    static constexpr bool enabled = true;  // Pruefling-Variante: aktiv wenn der Slot geladen ist

    /// SONDERFALL: NICHT leaf-only — zählt ALLE Knoten (Anti-Pattern, F15-Vergleichsbasis).
    [[nodiscard]] static constexpr bool             is_leaf_only() noexcept { return false; }
    [[nodiscard]] static constexpr std::string_view name()        noexcept { return "prtart_per_node_counter"; }
    [[nodiscard]] static constexpr std::string_view family_name() noexcept { return "PrtArtPerNodeCounter (F15 anti-pattern: per-node counter, cache-line ping-pong baseline)"; }
    [[nodiscard]] static constexpr std::string_view flag_suffix() noexcept { return "PRTART_PER_NODE"; }

    // prt-art-Algorithmus-Logik (PerNodeCounter) — forwarded.
    void record_access(std::uint64_t node, bool is_leaf) { counter_.record_access(node, is_leaf); }
    [[nodiscard]] std::uint64_t access_count(std::uint64_t node) const { return counter_.access_count(node); }
    [[nodiscard]] std::size_t   tracked_nodes()  const noexcept { return counter_.tracked_nodes(); }
    [[nodiscard]] std::uint64_t total_accesses() const { return counter_.total_accesses(); }
    void reset() { counter_.reset(); }

private:
    ::comdare::prt_art::telemetry::PerNodeCounter<std::uint64_t> counter_{};
};

/// Slot — pruefling_merge-konformer axis_11-Slot (SearchAlgorithm-Gattung).
struct Slot {
    using PrueflingVariants = mp::mp_list<PrtArtPerNodeCounter>;
    static constexpr bool has_pruefling = true;
    static constexpr ::comdare::cache_engine::anatomy::AnatomyGenus genus =
        ::comdare::cache_engine::anatomy::AnatomyGenus::SearchAlgorithm;
};

static_assert(ce11::concepts::TelemetryStrategy<PrtArtPerNodeCounter>);
static_assert(ce11::concepts::CacheEnginePermutationStrategy<PrtArtPerNodeCounter>);
static_assert(pf::PrueflingSlotConcept<Slot>);
static_assert(pf::HasPruefling_v<Slot>);

}  // namespace comdare::prt_art::slots::axis_11
