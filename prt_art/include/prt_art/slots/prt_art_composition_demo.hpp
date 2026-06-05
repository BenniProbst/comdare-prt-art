#pragma once
// V41.F.6.1 F.5-Schritt — prt-art-Slot-Composition (codegen-fähige volle Anatomie).
//
// Brücke Slot-Füllung → Codegen-Einheit: eine vollständige 17-Achsen-Composition, die prt-arts
// Slot-Wrapper (prefetch + value_handle) in den CE-Default-Anatomie-Rahmen (ArtComposition als Basis)
// einsetzt. Erfüllt IsComposition + HasCompositionLocation → ist damit eine vom anatomy_codegen_tool
// MATERIALISIERBARE Komposition (die Einheit, die comdare_perms_pa pro gemergter Permutation codegen
// würde). Beweist: prt-art-Slots komponieren in die volle SearchAlgorithmAnatomy, nicht nur in
// isolierte Achsen-Merges.
//
// HINWEIS: Dies ist eine Slot-DEMONSTRATION (prt-art-Wrapper in CE-Rahmen), NICHT die literale
// REV6-PRT-ART-Identität (deren prefetch=HotPath|AdaptiveDistance, telemetry=PathRead|Probability
// hätten eigene Slot-Wrapper, hier noch nicht angelegt). Telemetry bleibt bewusst CE-Default
// (LeafOnlyCounter) — der prt-art PerNodeCounter ist das F15-Anti-Pattern, nicht die Identität.

#include <compositions/art_reference.hpp>                  // CE-Default-Achsen (Anatomie-Rahmen)
#include <anatomy/composition_concept.hpp>                 // IsComposition + COMDARE_DEFINE_COMPOSITION_LOCATION
#include <prt_art/slots/axis_07_prefetch_slot.hpp>         // PrtArtRedirectPrefetch
#include <prt_art/slots/axis_14_value_handle_slot.hpp>     // PrtArtChainRefHandle

#include <string_view>

namespace comdare::prt_art::slots {

/// PrtArtCompositionDemo — 17-Achsen-Composition: CE-Defaults (ArtComposition) + prt-art-Slots
/// (prefetch + value_handle). Codegen-fähige Einheit (IsComposition + HasCompositionLocation).
struct PrtArtCompositionDemo {
    using Base = ::comdare::cache_engine::compositions::ArtComposition;

    // 15 Achsen aus dem CE-Default-Rahmen übernommen
    using search_algo        = Base::search_algo;
    using cache_traversal    = Base::cache_traversal;
    using mapping            = Base::mapping;
    using path_compression   = Base::path_compression;
    using node_type          = Base::node_type;
    using memory_layout      = Base::memory_layout;
    using allocator          = Base::allocator;
    using concurrency        = Base::concurrency;
    using serialization      = Base::serialization;
    using telemetry          = Base::telemetry;          // CE-Default (LeafOnly), NICHT PerNode-Anti-Pattern
    using isa                = Base::isa;
    using index_organization = Base::index_organization;
    using io_dispatch        = Base::io_dispatch;
    using migration_policy   = Base::migration_policy;
    using filter             = Base::filter;
    using queuing_q1         = Base::queuing_q1;   // T17 (Doc 30 §8.0: mandatorische SA-Achse, 19-Slot-Composition)
    using queuing_q2         = Base::queuing_q2;   // T18

    // 2 Achsen durch prt-art-Slot-Wrapper ersetzt (optional_prt_art_impl)
    using prefetch           = ::comdare::prt_art::slots::axis_07::PrtArtRedirectPrefetch;
    using value_handle       = ::comdare::prt_art::slots::axis_14::PrtArtChainRefHandle;

    static constexpr std::string_view name        = "PrtArtCompositionDemo";
    static constexpr std::string_view paper_id    = "prt-art REV6 §5.17 (Pruefling-Slot-Demonstration)";
    static constexpr std::string_view paper_title = "PRT-ART slot composition into full CacheEngine anatomy";

    COMDARE_DEFINE_COMPOSITION_LOCATION(
        "::comdare::prt_art::slots::PrtArtCompositionDemo",
        "prt_art/slots/prt_art_composition_demo.hpp");
};

// Compile-Time-Garantien: codegen-fähige Composition (das prüft das anatomy_codegen_tool).
static_assert(::comdare::cache_engine::anatomy::IsComposition<PrtArtCompositionDemo>);
static_assert(::comdare::cache_engine::anatomy::HasCompositionLocation<PrtArtCompositionDemo>);

}  // namespace comdare::prt_art::slots
