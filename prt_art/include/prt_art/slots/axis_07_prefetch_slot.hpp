#pragma once
// V41.F.6.1 Phase B — prt-art axis_07 Prefetch Slot (Plugin-Controller Slot-Füllung).
//
// Erste end-to-end-Demonstration der optional_prt_art_impl-Slot-Füllung: prt-art liefert
// einen CE-konformen Achsen-Wrapper (PrtArtRedirectPrefetch, §2.3 PF4-Kandidat) + einen
// pruefling_merge-Slot. Die cache-engine kann diese Variante via Stufe-2 (ERSETZT-mit-Fallback)
// oder Stufe-3 (full-join) in ihren COMPILE-TIME-Permutationsraum aufnehmen — rein
// metaprogrammatisch (mp11-Typ-Listen, KEINE Runtime-Selektion).
//
// Algorithmus: prt-arts RedirectPrefetch (RedirectNode-Subtree Fan-Out-Prefetch, REV 6 §5.17).
//
// Wird im Plugin-Controller-Build kompiliert (cache-engine -DCOMDARE_CE_PRUEFLINGE=<prt-art>),
// wo die CE-Achsen-Header unter topics/ + die generated Flags verfügbar sind.

#include <topics/prefetch/axis_07_prefetch/axis_07_prefetch_strategy_base.hpp>
#include <topics/prefetch/concepts/topic_prefetch_concept.hpp>
#include <topics/prefetch/axis_07_prefetch/concepts/axis_07_prefetch_concept.hpp>
#include <topics/prefetch/axis_07_prefetch/concepts/axis_07_prefetch_cache_engine_permutation_concept.hpp>
#include <anatomy/pruefling_merge.hpp> // PrueflingSlot-Pattern + AnatomyGenus (via anatomy_base)
#include <anatomy/organ_location.hpp>  // INC-B/R-B: COMDARE_DEFINE_ORGAN_LOCATION (per-Organ-Registry-Lokation)

#include <prt_art/prefetch/redirect_prefetch.hpp> // prt-art-Algorithmus-Logik

#include <boost/mp11.hpp>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace comdare::prt_art::slots::axis_07 {

namespace ce07 = ::comdare::cache_engine::prefetch::axis_07_prefetch;
namespace pf   = ::comdare::cache_engine::anatomy::pruefling;
namespace mp   = ::boost::mp11;

/// prt-art-eigene Subaxis-Markierung (PF4 redirect-subtree; §2.3 optional_prt_art_impl).
/// Concept verlangt nur, dass `axis_tag` ein Typ ist — Prüfling-Tags sind erlaubt.
struct redirect_subtree_tag {};

/// PrtArtRedirectPrefetch — CE-konformer axis_07-Wrapper über prt-arts RedirectPrefetch.
/// Erfüllt PrefetchStrategy + CacheEnginePermutationStrategy → via pruefling_merge in den
/// CE-Permutationsraum einsetzbar (Stufe 2/3).
class PrtArtRedirectPrefetch : public ce07::PrefetchStrategyBase<PrtArtRedirectPrefetch> {
public:
    using topic_tag = ::comdare::cache_engine::prefetch::concepts::PrefetchTopicTag;
    using axis_tag  = redirect_subtree_tag;
    using family_id = std::integral_constant<int, 4>; // PF4 (prt-art-Variante)

    static constexpr bool enabled = true; // Pruefling-Variante: aktiv wenn der Slot geladen ist

    [[nodiscard]] static constexpr bool             is_active() noexcept { return true; }
    [[nodiscard]] static constexpr std::string_view name() noexcept { return "prtart_redirect_prefetch"; }
    [[nodiscard]] static constexpr std::string_view family_name() noexcept {
        return "PrtArtRedirectPrefetch (RedirectNode-subtree fan-out prefetch, prt-art REV6 §5.17)";
    }
    [[nodiscard]] static constexpr std::string_view flag_suffix() noexcept { return "PRTART_REDIRECT"; }

    // W-B (2026-08-15): Pflicht-algo_version je Kompositions-Organ-Variante (ce Bauplan par. 2, CRTP-Ctor-Guard
    // der PrefetchStrategyBase seit S-1/A1). Segment-Format "<achse>=<name()>@<X.Y.Z[flag]>"; Flag 'p' =
    // prt-art-Pruefling-Provenienz (distinkt zum ce-Flag 'c', pruefling_merge-Kollisionswache). Startwert;
    // Bump bei algorithmischer Aenderung dieser Variante (ce-Konvention, axis_14_value_handle_chain_ref.hpp).
    static constexpr std::string_view algo_version = "1.0.0.p";

    // INC-B/R-B (2026-07-14): Per-Organ-Registry-Lokation (FQ-Typ + Header). Befuellt die 'type'/'header'-
    // Attribute des prt-art-Registry-Generators (prt_art_axis_registry.xml, gleiches Schema wie die ce-Registry).
    COMDARE_DEFINE_ORGAN_LOCATION("::comdare::prt_art::slots::axis_07::PrtArtRedirectPrefetch",
                                  "prt_art/slots/axis_07_prefetch_slot.hpp");

    static constexpr std::uint8_t kFanOut = ::comdare::prt_art::prefetch::RedirectPrefetch::kFanOut;

    // prt-art-Algorithmus-Logik (RedirectPrefetch) — forwarded.
    void                        schedule(std::uint64_t terminal_addr) noexcept { impl_.schedule(terminal_addr); }
    [[nodiscard]] std::uint64_t slot(std::uint8_t i) const noexcept { return impl_.slot(i); }
    [[nodiscard]] std::uint64_t total_scheduled() const noexcept { return impl_.total_scheduled(); }
    void                        reset() noexcept { impl_.reset(); }

private:
    ::comdare::prt_art::prefetch::RedirectPrefetch impl_{};
};

/// Slot — pruefling_merge-konformer axis_07-Slot (SearchAlgorithm-Gattung).
/// has_pruefling=true → Stufe-2 ERSETZT die CE-Defaults; Stufe-3 nimmt die Variante zusätzlich auf.
struct Slot {
    using PrueflingVariants                                                       = mp::mp_list<PrtArtRedirectPrefetch>;
    static constexpr bool                                           has_pruefling = true;
    static constexpr ::comdare::cache_engine::anatomy::AnatomyGenus genus =
        ::comdare::cache_engine::anatomy::AnatomyGenus::SearchAlgorithm;
};

// Compile-Time-Konformanz-Garantien (greifen nur zur Compile-Time → beweisen Metaprogramm-Natur).
static_assert(ce07::concepts::PrefetchStrategy<PrtArtRedirectPrefetch>);
static_assert(ce07::concepts::CacheEnginePermutationStrategy<PrtArtRedirectPrefetch>);
static_assert(pf::PrueflingSlotConcept<Slot>);
static_assert(pf::HasPruefling_v<Slot>);

} // namespace comdare::prt_art::slots::axis_07
