#pragma once
// V41.F.6.1 Phase B — prt-art axis_14 Value-Handle Slot (Plugin-Controller Slot-Füllung).
//
// Dritte Slot-Füllung (nach axis_07 prefetch + axis_01 page-type): prt-art liefert einen
// CE-konformen VALUE-HANDLE-Wrapper (PrtArtChainRefHandle) mit der konkreten Linked-List-
// Verwaltung für Multi-Value-Schlüssel (Doku 19 §3.10 designierter optional_prt_art_impl —
// die CE-Achse axis_14 beschreibt nur die Wahl "chained external", die Pool-Offset-Laufzeit
// ist prt-art-Detail). Wird via pruefling_merge ZUR COMPILE-TIME in den CE-Permutationsraum gemergt.

#include <topics/value_handle/axis_14_value_handle/axis_14_value_handle_strategy_base.hpp>
#include <topics/value_handle/axis_14_value_handle/concepts/axis_14_value_handle_concept.hpp>
#include <topics/value_handle/axis_14_value_handle/concepts/axis_14_value_handle_cache_engine_permutation_concept.hpp>
#include <topics/value_handle/concepts/topic_value_handle_concept.hpp>
#include <anatomy/pruefling_merge.hpp> // PrueflingSlot-Pattern + AnatomyGenus
#include <anatomy/organ_location.hpp>  // INC-B/R-B: COMDARE_DEFINE_ORGAN_LOCATION (per-Organ-Registry-Lokation)

#include <prt_art/value_handle/chain_ref_handle.hpp> // prt-art-Algorithmus-Logik

#include <boost/mp11.hpp>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace comdare::prt_art::slots::axis_14 {

namespace ce14 = ::comdare::cache_engine::value_handle::axis_14_value_handle;
namespace pf   = ::comdare::cache_engine::anatomy::pruefling;
namespace mp   = ::boost::mp11;

/// prt-art-eigene Subaxis-Markierung (chained-external storage-location).
struct chained_external_tag {};

/// PrtArtChainRefHandle — CE-konformer axis_14-VALUE-HANDLE-Wrapper über prt-arts ChainRefHandle.
/// Trägt die konkrete Linked-List-Verwaltung (chain_head_offset/length, prepend_node) für
/// Multi-Value-Schlüssel. Erfüllt ValueHandleStrategy + CacheEnginePermutationStrategy → via
/// pruefling_merge in den CE-Permutationsraum einsetzbar (Stufe 2/3).
class PrtArtChainRefHandle : public ce14::ValueHandleStrategyBase<PrtArtChainRefHandle> {
public:
    using topic_tag = ::comdare::cache_engine::value_handle::concepts::ValueHandleTopicTag;
    using axis_tag  = chained_external_tag;
    using family_id = std::integral_constant<int, 15>; // prt-art VH-Range (distinkt zu CE 1-5)

    static constexpr bool enabled = true; // Pruefling-Variante: aktiv wenn der Slot geladen ist

    [[nodiscard]] static constexpr bool             is_inline() noexcept { return false; } // chained external
    [[nodiscard]] static constexpr std::string_view name() noexcept { return "prtart_chain_ref_handle"; }
    [[nodiscard]] static constexpr std::string_view family_name() noexcept {
        return "PrtArtChainRefHandle (multi-value linked-list pool reference, prt-art REV6 §5.4)";
    }
    [[nodiscard]] static constexpr std::string_view flag_suffix() noexcept { return "PRTART_CHAIN_REF"; }

    // W-B (2026-08-15): Pflicht-algo_version je Kompositions-Organ-Variante (ce Bauplan par. 2, CRTP-Ctor-Guard
    // der ValueHandleStrategyBase seit S-1/A1). Segment-Format "<achse>=<name()>@<X.Y.Z[flag]>"; Flag 'p' =
    // prt-art-Pruefling-Provenienz (distinkt zum ce-Flag 'c', pruefling_merge-Kollisionswache). Startwert;
    // Bump bei algorithmischer Aenderung dieser Variante (ce-Konvention, axis_14_value_handle_chain_ref.hpp).
    static constexpr std::string_view algo_version = "1.0.0.p";

    // INC-B/R-B (2026-07-14): Per-Organ-Registry-Lokation (FQ-Typ + Header). Befuellt die 'type'/'header'-
    // Attribute des prt-art-Registry-Generators (prt_art_axis_registry.xml, gleiches Schema wie die ce-Registry).
    COMDARE_DEFINE_ORGAN_LOCATION("::comdare::prt_art::slots::axis_14::PrtArtChainRefHandle",
                                  "prt_art/slots/axis_14_value_handle_slot.hpp");

    // prt-art-Algorithmus-Logik (ChainRefHandle Linked-List-Verwaltung) — forwarded.
    [[nodiscard]] std::uint64_t chain_head_offset() const noexcept { return handle_.chain_head_offset(); }
    [[nodiscard]] std::uint32_t chain_length() const noexcept { return handle_.chain_length(); }
    [[nodiscard]] bool          is_empty() const noexcept { return handle_.is_empty(); }
    void prepend_node(std::uint64_t new_head_offset) noexcept { handle_.prepend_node(new_head_offset); }
    void clear() noexcept { handle_.clear(); }

private:
    ::comdare::prt_art::value_handle::ChainRefHandle handle_{};
};

/// Slot — pruefling_merge-konformer axis_14-Slot (SearchAlgorithm-Gattung).
struct Slot {
    using PrueflingVariants                                                       = mp::mp_list<PrtArtChainRefHandle>;
    static constexpr bool                                           has_pruefling = true;
    static constexpr ::comdare::cache_engine::anatomy::AnatomyGenus genus =
        ::comdare::cache_engine::anatomy::AnatomyGenus::SearchAlgorithm;
};

static_assert(ce14::concepts::ValueHandleStrategy<PrtArtChainRefHandle>);
static_assert(ce14::concepts::CacheEnginePermutationStrategy<PrtArtChainRefHandle>);
static_assert(pf::PrueflingSlotConcept<Slot>);
static_assert(pf::HasPruefling_v<Slot>);

} // namespace comdare::prt_art::slots::axis_14
