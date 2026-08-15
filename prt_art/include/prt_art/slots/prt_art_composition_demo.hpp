#pragma once
// V41.F.6.1 F.5-Schritt — prt-art-Slot-Composition (codegen-fähige volle Anatomie).
//
// Brücke Slot-Füllung → Codegen-Einheit: eine vollständige 18-Achsen-Composition, die prt-arts
// Slot-Wrapper (prefetch + value_handle) in den CE-Default-Anatomie-Rahmen (ArtComposition als Basis)
// einsetzt. Erfüllt IsComposition + HasCompositionLocation → ist damit eine vom anatomy_codegen_tool
// MATERIALISIERBARE Komposition (die Einheit, die comdare_perms_pa pro gemergter Permutation codegen
// würde). Beweist: prt-art-Slots komponieren in die volle SearchAlgorithmAnatomy, nicht nur in
// isolierte Achsen-Merges.
//
// HINWEIS: Dies ist eine Slot-DEMONSTRATION (prt-art-Wrapper in CE-Rahmen), NICHT die literale
// REV6-PRT-ART-Identität (deren prefetch=HotPath|AdaptiveDistance, telemetry=PathRead|Probability
// hätten eigene Slot-Wrapper, hier noch nicht angelegt). Telemetry ist seit ce Bau-INC-2c
// System-Achse und KEIN Kompositions-Slot mehr (Alias ersatzlos entfallen, s. Vermerk unten bei
// den using-Zeilen) -- ein telemetry-Slot ist auf Kompositions-Ebene gegenstandslos; der
// LeafOnly/PerNode-Kontrast (prt-art PerNodeCounter = F15-Anti-Pattern) lebt auf der
// Achsen-Merge-Ebene weiter.

#include <compositions/art_reference.hpp>              // CE-Default-Achsen (Anatomie-Rahmen)
#include <anatomy/composition_concept.hpp>             // IsComposition + COMDARE_DEFINE_COMPOSITION_LOCATION
#include <prt_art/slots/axis_07_prefetch_slot.hpp>     // PrtArtRedirectPrefetch
#include <prt_art/slots/axis_14_value_handle_slot.hpp> // PrtArtChainRefHandle

#include <string_view>

namespace comdare::prt_art::slots {

/// PrtArtCompositionDemo -- 18-Achsen-Composition (W-B 2026-08-15, ce-Stand ORG-18): CE-Defaults
/// (ArtComposition, 16 von 18) + prt-art-Slots (prefetch + value_handle, 2 von 18). Codegen-faehige
/// Einheit (IsComposition + HasCompositionLocation).
struct PrtArtCompositionDemo {
    using Base = ::comdare::cache_engine::compositions::ArtComposition;

    // 16 Achsen aus dem CE-Default-Rahmen uebernommen
    using search_algo      = Base::search_algo;
    using cache_traversal  = Base::cache_traversal;
    using mapping          = Base::mapping;
    using path_compression = Base::path_compression;
    using node_type        = Base::node_type;
    using memory_layout    = Base::memory_layout;
    using allocator        = Base::allocator;
    using concurrency      = Base::concurrency;
    using serialization    = Base::serialization;
    // W-B (2026-08-15): telemetry + isa sind KEINE Kompositions-Slots mehr (ce Bau-INC-2c: telemetry =
    // System-Achse; Bau-INC-2d/ABI-6: isa = Target-ISA-System-Achse) -- Aliase ersatzlos entfallen.
    using index_organization = Base::index_organization;
    using io_dispatch        = Base::io_dispatch;
    using migration_policy   = Base::migration_policy;
    using filter             = Base::filter;
    using queuing_q1         = Base::queuing_q1; // T17 (Doc 30 par. 8.0: mandatorische SA-Achse)
    using queuing_q2         = Base::queuing_q2; // T18
    // W-B (2026-08-15): 18. Organ-Slot persistence_target (ce STRUKT-R ORG-18, Pflicht ohne Default).
    using persistence_target = Base::persistence_target;

    // 2 Achsen durch prt-art-Slot-Wrapper ersetzt (optional_prt_art_impl)
    using prefetch     = ::comdare::prt_art::slots::axis_07::PrtArtRedirectPrefetch;
    using value_handle = ::comdare::prt_art::slots::axis_14::PrtArtChainRefHandle;

    static constexpr std::string_view name        = "PrtArtCompositionDemo";
    static constexpr std::string_view paper_id    = "prt-art REV6 §5.17 (Pruefling-Slot-Demonstration)";
    static constexpr std::string_view paper_title = "PRT-ART slot composition into full CacheEngine anatomy";

    COMDARE_DEFINE_COMPOSITION_LOCATION("::comdare::prt_art::slots::PrtArtCompositionDemo",
                                        "prt_art/slots/prt_art_composition_demo.hpp");
};

// Compile-Time-Garantien: codegen-fähige Composition (das prüft das anatomy_codegen_tool).
static_assert(::comdare::cache_engine::anatomy::IsComposition<PrtArtCompositionDemo>);
static_assert(::comdare::cache_engine::anatomy::HasCompositionLocation<PrtArtCompositionDemo>);

} // namespace comdare::prt_art::slots
