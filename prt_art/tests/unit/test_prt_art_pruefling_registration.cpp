// V41.E11 Phase B — prt-art Pruefling-Registrierung Integration-Test.
// Wird NUR im Plugin-Controller-Build gebaut (cache-engine mit COMDARE_CE_PRUEFLINGE=<prt-art>),
// via comdare_pruefling.cmake (das comdare_add_test aufruft). Verifiziert end-to-end, dass
// die cache-engine prt-art als Plugin laedt + dessen Factory registriert + Prueflinge erzeugt.

#include <gtest/gtest.h>

#include <prt_art/identity/prt_art_pruefling_factory.hpp>
#include <cache_engine/api/pruefling_registry.hpp>

// V41.F.6.1 Phase B — Slot-Füllung-Demonstration (compile-time merge)
#include <prt_art/slots/axis_07_prefetch_slot.hpp>
#include <prt_art/slots/axis_01_page_type_slot.hpp>
#include <prt_art/slots/axis_14_value_handle_slot.hpp>
#include <prt_art/slots/axis_11_telemetry_slot.hpp>
#include <topics/prefetch/axis_07_prefetch/axis_07_prefetch_registry.hpp>
#include <topics/nodes/axis_01_page_type/axis_01_page_type_registry.hpp>
#include <topics/value_handle/axis_14_value_handle/axis_14_value_handle_registry.hpp>
#include <topics/telemetry/axis_11_telemetry/axis_11_telemetry_registry.hpp>
#include <anatomy/pruefling_merge.hpp>
#include <src/permutations/permutation_engine.hpp>   // F.5: 3-Stufen-Permutations-Count
#include <prt_art/slots/prt_art_composition_demo.hpp>  // F.5: codegen-fähige prt-art-Composition
#include <anatomy/composition_concept.hpp>
#include <anatomy/search_algorithm_anatomy.hpp>
#include <boost/mp11.hpp>

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace api = ::comdare::cache_engine::api;
namespace ppf = ::comdare::prt_art::pruefling;

TEST(E11_PrtArtPruefling, RegistersIntoCacheEngineRegistry) {
    api::PrueflingRegistry reg;
    ppf::register_prt_art_pruefling(reg);
    ASSERT_EQ(reg.size(), 1u);
    auto* f = reg.find("prt-art");
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->pruefling_name(), std::string_view{"prt-art"});
    EXPECT_GE(f->available_axes_combinations().size(), 1u);
}

TEST(E11_PrtArtPruefling, FactoryCreatesRunnablePruefling) {
    ppf::PrtArtPrueflingFactory factory;
    auto p = factory.create("page=redirect|node=bplus|vh=inline");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->name(), std::string_view{"prt-art"});
    EXPECT_EQ(p->axes_signature(), std::string_view{"page=redirect|node=bplus|vh=inline"});
    double micros = -1.0;
    EXPECT_EQ(p->run(1000, micros), 0);
    EXPECT_GT(micros, 0.0);
}

TEST(E11_PrtArtPruefling, ZeroOpsIsZeroLatency) {
    ppf::PrtArtPruefling p{"x"};
    double micros = -1.0;
    EXPECT_EQ(p.run(0, micros), 0);
    EXPECT_EQ(micros, 0.0);
}

// =================================================================
// V41.F.6.1 Phase B — Slot-Füllung-Demonstration (axis_07 prefetch)
//
// Beweist: prt-art liefert eine CE-konforme Achsen-Variante, die der CE-
// Permutation-Merge (pruefling_merge.hpp) zur COMPILE-TIME aufnimmt. Alle
// Merge-Prüfungen sind static_assert → greifen ausschließlich auf Compile-Time-
// Typ-Berechnung (mp11), KEINE Runtime-Selektion (Beleg für Metaprogramm-Natur).
// =================================================================

namespace pf07   = ::comdare::cache_engine::anatomy::pruefling;
namespace slot07 = ::comdare::prt_art::slots::axis_07;
namespace ce07   = ::comdare::cache_engine::prefetch::axis_07_prefetch;
namespace mp11   = ::boost::mp11;

TEST(PhaseB_Axis07Slot, WrapperConformsToCeConcepts) {
    static_assert(ce07::concepts::PrefetchStrategy<slot07::PrtArtRedirectPrefetch>);
    static_assert(ce07::concepts::CacheEnginePermutationStrategy<slot07::PrtArtRedirectPrefetch>);
    SUCCEED();
}

TEST(PhaseB_Axis07Slot, SlotIsPrueflingConformAndPopulated) {
    static_assert(pf07::PrueflingSlotConcept<slot07::Slot>);
    static_assert(pf07::HasPruefling_v<slot07::Slot>);
    // Gattungs-Constraint: SearchAlgorithm (kreuzbar mit CE-SearchAlgorithm-Achsen).
    static_assert(pf07::slot_genus_v<slot07::Slot>
                  == ::comdare::cache_engine::anatomy::AnatomyGenus::SearchAlgorithm);
    SUCCEED();
}

TEST(PhaseB_Axis07Slot, StufeTwoReplacesCeDefaults) {
    // Stufe 2 (comdare_perms_pa): prt-art-Variante ERSETZT die CE-Defaults für axis_07.
    using Merged = pf07::StufeTwoAxis<ce07::AllPrefetchers, slot07::Slot>;
    static_assert(std::is_same_v<Merged, mp11::mp_list<slot07::PrtArtRedirectPrefetch>>);
    SUCCEED();
}

TEST(PhaseB_Axis07Slot, StufeThreeFullJoinUnionsBoth) {
    // Stufe 3 (comdare_perms_full_join): CE-Defaults + prt-art-Variante, dedupliziert.
    using Joined = pf07::StufeThreeAxis<ce07::AllPrefetchers, slot07::Slot>;
    static_assert(mp11::mp_contains<Joined, slot07::PrtArtRedirectPrefetch>::value);
    static_assert(mp11::mp_contains<Joined, ce07::NonePrefetch>::value);
    static_assert(mp11::mp_contains<Joined, ce07::DistanceEstimatorPrefetch>::value);
    static_assert(mp11::mp_size<Joined>::value
                  == mp11::mp_size<ce07::AllPrefetchers>::value + 1);
    SUCCEED();
}

TEST(PhaseB_Axis07Slot, MergeAxisDispatchMatchesDirectStufen) {
    // MergeAxis-Dispatch (Non-Type-Template-Param, kein Runtime-Switch) == direkte Stufen.
    using ViaDispatch2 = pf07::MergeAxis<pf07::MergeStrategy::Stufe2_PrueflingReplace,
                                         ce07::AllPrefetchers, slot07::Slot>;
    using ViaDispatch3 = pf07::MergeAxis<pf07::MergeStrategy::Stufe3_FullJoin,
                                         ce07::AllPrefetchers, slot07::Slot>;
    static_assert(std::is_same_v<ViaDispatch2,
                                 pf07::StufeTwoAxis<ce07::AllPrefetchers, slot07::Slot>>);
    static_assert(std::is_same_v<ViaDispatch3,
                                 pf07::StufeThreeAxis<ce07::AllPrefetchers, slot07::Slot>>);
    SUCCEED();
}

TEST(PhaseB_Axis07Slot, PrtArtAlgorithmLogicForwarded) {
    // Die migrierte prt-art-Logik (RedirectPrefetch fan-out) funktioniert im CE-Wrapper.
    slot07::PrtArtRedirectPrefetch p{};
    p.schedule(std::uint64_t{0x1000});
    EXPECT_EQ(p.slot(0), 0x1000u);          // Hauptziel
    EXPECT_EQ(p.slot(1), 0x1000u + 64u);    // +1 Cache-Line
    EXPECT_EQ(p.slot(2), 0x1000u - 64u);    // -1 Cache-Line
    EXPECT_EQ(p.total_scheduled(), 1u);
    EXPECT_EQ(slot07::PrtArtRedirectPrefetch::kFanOut, 3);
}

// =================================================================
// V41.F.6.1 Phase B — 2. Slot-Füllung (axis_01 PAGE-TYPE, Pattern-Replikation)
// =================================================================

namespace slot01 = ::comdare::prt_art::slots::axis_01;
namespace ce01   = ::comdare::cache_engine::nodes::axis_01_page_type;

TEST(PhaseB_Axis01Slot, WrapperConformsToCeConcepts) {
    static_assert(ce01::concepts::PageTypeStrategy<slot01::PrtArtBPlusPageType>);
    static_assert(ce01::concepts::CacheEnginePermutationStrategy<slot01::PrtArtBPlusPageType>);
    // Seitentyp-Klassifikation: BPlus-Verzweigungsseite.
    static_assert(slot01::PrtArtBPlusPageType::page_kind() == ce01::concepts::PageKind::BPlus);
    static_assert(slot01::PrtArtBPlusPageType::is_branch());
    SUCCEED();
}

TEST(PhaseB_Axis01Slot, StufeTwoReplacesCeDefaults) {
    using Merged = pf07::StufeTwoAxis<ce01::AllPageTypes, slot01::Slot>;
    static_assert(std::is_same_v<Merged, mp11::mp_list<slot01::PrtArtBPlusPageType>>);
    SUCCEED();
}

TEST(PhaseB_Axis01Slot, StufeThreeFullJoinUnionsBoth) {
    using Joined = pf07::StufeThreeAxis<ce01::AllPageTypes, slot01::Slot>;
    static_assert(mp11::mp_contains<Joined, slot01::PrtArtBPlusPageType>::value);
    static_assert(mp11::mp_contains<Joined, ce01::BPlusPageType>::value);       // CE-Default bleibt
    static_assert(mp11::mp_contains<Joined, ce01::RedirectPageType>::value);
    static_assert(mp11::mp_size<Joined>::value
                  == mp11::mp_size<ce01::AllPageTypes>::value + 1);
    SUCCEED();
}

TEST(PhaseB_Axis01Slot, DensityDrivenDispatchIsDiplomaCore) {
    // prt-arts Density-Dispatch (25/50/75%-Schwellen) im CE-Wrapper.
    using K = slot01::PrtArtBPlusPageType::InternalSearchKind;
    slot01::PrtArtBPlusPageType page{};
    EXPECT_EQ(page.key_count(), 0u);
    EXPECT_EQ(page.recommended_kind(), K::Array256);   // leer → Array256 (Density 0%)
    // 200/256 ≈ 78% → VectorU16U16 (>=75%).
    for (std::uint64_t i = 0; i < 200; ++i)
        page.insert_slot(static_cast<std::uint8_t>(i & 0xFF), i);
    EXPECT_EQ(page.key_count(), 200u);
    EXPECT_GT(page.density_percent(), 75.0);
    EXPECT_EQ(page.recommended_kind(), K::VectorU16U16);
}

// =================================================================
// V41.F.6.1 Phase B — 3. Slot-Füllung (axis_14 VALUE-HANDLE, Pattern-Replikation)
// =================================================================

namespace slot14 = ::comdare::prt_art::slots::axis_14;
namespace ce14   = ::comdare::cache_engine::value_handle::axis_14_value_handle;

TEST(PhaseB_Axis14Slot, WrapperConformsToCeConcepts) {
    static_assert(ce14::concepts::ValueHandleStrategy<slot14::PrtArtChainRefHandle>);
    static_assert(ce14::concepts::CacheEnginePermutationStrategy<slot14::PrtArtChainRefHandle>);
    static_assert(!slot14::PrtArtChainRefHandle::is_inline());  // chained external
    SUCCEED();
}

TEST(PhaseB_Axis14Slot, StufeTwoReplacesCeDefaults) {
    using Merged = pf07::StufeTwoAxis<ce14::AllHandles, slot14::Slot>;
    static_assert(std::is_same_v<Merged, mp11::mp_list<slot14::PrtArtChainRefHandle>>);
    SUCCEED();
}

TEST(PhaseB_Axis14Slot, StufeThreeFullJoinUnionsBoth) {
    using Joined = pf07::StufeThreeAxis<ce14::AllHandles, slot14::Slot>;
    static_assert(mp11::mp_contains<Joined, slot14::PrtArtChainRefHandle>::value);
    static_assert(mp11::mp_contains<Joined, ce14::ChainRefValueHandle>::value);  // CE VH5 bleibt
    static_assert(mp11::mp_size<Joined>::value
                  == mp11::mp_size<ce14::AllHandles>::value + 1);
    SUCCEED();
}

TEST(PhaseB_Axis14Slot, LinkedListManagementForwarded) {
    // prt-arts Linked-List-Verwaltung (Multi-Value-Chain) im CE-Wrapper.
    slot14::PrtArtChainRefHandle h{};
    EXPECT_TRUE(h.is_empty());
    h.prepend_node(0x2000);
    h.prepend_node(0x3000);
    EXPECT_FALSE(h.is_empty());
    EXPECT_EQ(h.chain_head_offset(), 0x3000u);  // letztes prepend ist Head
    EXPECT_EQ(h.chain_length(), 2u);
    h.clear();
    EXPECT_TRUE(h.is_empty());
    EXPECT_EQ(h.chain_length(), 0u);
}

// =================================================================
// V41.F.6.1 Phase B — 4. Slot-Füllung (axis_11 TELEMETRY, F15-Anti-Pattern)
// =================================================================

namespace slot11 = ::comdare::prt_art::slots::axis_11;
namespace ce11   = ::comdare::cache_engine::telemetry::axis_11_telemetry;

TEST(PhaseB_Axis11Slot, WrapperConformsToCeConcepts) {
    static_assert(ce11::concepts::TelemetryStrategy<slot11::PrtArtPerNodeCounter>);
    static_assert(ce11::concepts::CacheEnginePermutationStrategy<slot11::PrtArtPerNodeCounter>);
    static_assert(!slot11::PrtArtPerNodeCounter::is_leaf_only());  // zählt ALLE Knoten (Anti-Pattern)
    SUCCEED();
}

// F15-KERN: Stufe-3 full-join stellt LeafOnly (CE-Hauptvariante) + PerNode (prt-art-Anti-Pattern)
// im SELBEN Permutations-Sweep gegenüber → der Mess-Treiber kann beide vergleichen (Welch-Test).
TEST(PhaseB_Axis11Slot, StufeThreeUnionsLeafOnlyAndPerNodeForF15) {
    using Joined = pf07::StufeThreeAxis<ce11::AllTelemetries, slot11::Slot>;
    static_assert(mp11::mp_contains<Joined, slot11::PrtArtPerNodeCounter>::value);  // Anti-Pattern
    static_assert(mp11::mp_contains<Joined, ce11::LeafOnlyCounter>::value);          // CE-Hauptvariante
    static_assert(mp11::mp_size<Joined>::value
                  == mp11::mp_size<ce11::AllTelemetries>::value + 1);
    SUCCEED();
}

TEST(PhaseB_Axis11Slot, StufeTwoReplacesCeDefaults) {
    using Merged = pf07::StufeTwoAxis<ce11::AllTelemetries, slot11::Slot>;
    static_assert(std::is_same_v<Merged, mp11::mp_list<slot11::PrtArtPerNodeCounter>>);
    SUCCEED();
}

TEST(PhaseB_Axis11Slot, PerNodeCountsAllNodesIncludingInner) {
    // Anti-Pattern-Verhalten: zählt auch Inner-Nodes (is_leaf=false) — im Gegensatz zu LeafOnly.
    slot11::PrtArtPerNodeCounter c{};
    c.record_access(0x10, /*is_leaf=*/true);
    c.record_access(0x20, /*is_leaf=*/false);   // Inner-Node — PerNode zählt trotzdem
    c.record_access(0x20, /*is_leaf=*/false);
    EXPECT_EQ(c.access_count(0x10), 1u);
    EXPECT_EQ(c.access_count(0x20), 2u);         // Inner-Node mitgezählt (Ping-Pong-Risiko)
    EXPECT_EQ(c.tracked_nodes(), 2u);
    EXPECT_EQ(c.total_accesses(), 3u);
}

// =================================================================
// V41.F.6.1 F.5-Kern — 3-Stufen-PERMUTATIONS-RAUM über 4 echte Achsen (Compile-Time)
//
// Verdrahtet die bewiesene Slot-Füllung (4 Achsen) in die PermutationEngine: der kartesische
// Produkt-Raum jeder Stufe = die Binary-Set-Größe, die comdare_perms_ce/pa/full_join codegen würde.
// Vollständig ZUR COMPILE-TIME (count() ist constexpr) → genau der "Binaries aus Rekombinationen"-Kern.
// =================================================================

namespace pe = ::comdare::cache_engine::permutations;

// Stufe 1 — comdare_perms_ce (CE-only, KEINE Pruefling-Beteiligung)
struct TC07_S1 { using StaticAxisVariants = ce07::AllPrefetchers;  };
struct TC01_S1 { using StaticAxisVariants = ce01::AllPageTypes;    };
struct TC14_S1 { using StaticAxisVariants = ce14::AllHandles;      };
struct TC11_S1 { using StaticAxisVariants = ce11::AllTelemetries;  };

// Stufe 2 — comdare_perms_pa (Pruefling ERSETZT pro Achse → je 1 Variante → 1 Komposition)
struct TC07_S2 { using StaticAxisVariants = pf07::StufeTwoAxis<ce07::AllPrefetchers, slot07::Slot>; };
struct TC01_S2 { using StaticAxisVariants = pf07::StufeTwoAxis<ce01::AllPageTypes,   slot01::Slot>; };
struct TC14_S2 { using StaticAxisVariants = pf07::StufeTwoAxis<ce14::AllHandles,     slot14::Slot>; };
struct TC11_S2 { using StaticAxisVariants = pf07::StufeTwoAxis<ce11::AllTelemetries, slot11::Slot>; };

// Stufe 3 — comdare_perms_full_join (CE + Pruefling unioniert pro Achse)
struct TC07_S3 { using StaticAxisVariants = pf07::StufeThreeAxis<ce07::AllPrefetchers, slot07::Slot>; };
struct TC01_S3 { using StaticAxisVariants = pf07::StufeThreeAxis<ce01::AllPageTypes,   slot01::Slot>; };
struct TC14_S3 { using StaticAxisVariants = pf07::StufeThreeAxis<ce14::AllHandles,     slot14::Slot>; };
struct TC11_S3 { using StaticAxisVariants = pf07::StufeThreeAxis<ce11::AllTelemetries, slot11::Slot>; };

TEST(F5_DreigliedrigkeitPermutationSpace, ThreeStufenProduceDistinctBinarySetSizes) {
    using EngineS1 = pe::PermutationEngine<TC07_S1, TC01_S1, TC14_S1, TC11_S1>;
    using EngineS2 = pe::PermutationEngine<TC07_S2, TC01_S2, TC14_S2, TC11_S2>;
    using EngineS3 = pe::PermutationEngine<TC07_S3, TC01_S3, TC14_S3, TC11_S3>;

    constexpr std::size_t n07 = mp11::mp_size<ce07::AllPrefetchers>::value;
    constexpr std::size_t n01 = mp11::mp_size<ce01::AllPageTypes>::value;
    constexpr std::size_t n14 = mp11::mp_size<ce14::AllHandles>::value;
    constexpr std::size_t n11 = mp11::mp_size<ce11::AllTelemetries>::value;

    // Stufe 1: kartesisches Produkt der CE-Defaults (4 Achsen).
    static_assert(EngineS1::count() == n07 * n01 * n14 * n11);
    // Stufe 2 (TEILSICHT: NUR die 4 belegten Achsen): jede vom Pruefling ersetzt (je 1 Variante) → 1 Komposition.
    // ACHTUNG: das ist NICHT der volle Stufe-2-Raum! Ueber die volle Anatomie expandieren die ABSTRAKT-LEEREN
    // Achsen auf die volle CE-Liste (User 2026-05-30) — siehe StufeTwoEmptyAxesReuseAllCeAlgorithms unten.
    static_assert(EngineS2::count() == 1);
    // Stufe 3: jede Achse = CE + prt-art-Variante (distinkt, kein Dedup) → Produkt der (n+1).
    static_assert(EngineS3::count() == (n07 + 1) * (n01 + 1) * (n14 + 1) * (n11 + 1));
    // Die 3 Stufen spannen drei verschieden große Binary-Set-Räume auf.
    static_assert(EngineS3::count() > EngineS1::count());
    static_assert(EngineS1::count() > EngineS2::count());
    SUCCEED();
}

// =================================================================
// V41.F.6.1 F.5-Schärfung (User 2026-05-30) — Stufe-2 VOLL-ANATOMIE: die Regel der ABSTRAKT-LEEREN Achse.
//
// Eine Prüfling-Tier-Klassifizierung (z.B. prt-art) pinnt NUR ihre belegten Achsen. Jede Achse, deren Slot
// abstrakt leer ist (EmptyPrueflingSlot, has_pruefling=false), reust AUTOMATISCH ALLE CE-Algorithmen als
// Default und der Generator permutiert sie VOLL aus. → Der Stufe-2-Raum des "Tierklassen-Prototyps" ist das
// kartesische Produkt ÜBER DIE LEEREN ACHSEN (≠ 1), das das Prüf-Dock ausmisst, um die schnellste
// Rekombination zu finden. (Mengenlehre: B = ∏_belegt {prüfling_i} × ∏_leer A_j.)  Doku 24 §8.9.1.
// =================================================================

using ES = pf07::EmptyPrueflingSlot;

// Belegt → 1 Prüfling-Variante.  Leer (ES) → volle CE-DefaultList.
struct TC07_S2mix { using StaticAxisVariants = pf07::StufeTwoAxis<ce07::AllPrefetchers, slot07::Slot>; };  // BELEGT
struct TC01_S2mix { using StaticAxisVariants = pf07::StufeTwoAxis<ce01::AllPageTypes,   ES>;           };  // LEER
struct TC14_S2mix { using StaticAxisVariants = pf07::StufeTwoAxis<ce14::AllHandles,     ES>;           };  // LEER
struct TC11_S2mix { using StaticAxisVariants = pf07::StufeTwoAxis<ce11::AllTelemetries, slot11::Slot>; };  // BELEGT

TEST(F5_DreigliedrigkeitPermutationSpace, StufeTwoEmptyAxesReuseAllCeAlgorithms) {
    // (a) Type-Beleg der Regel: leere Achse ⇒ StufeTwoAxis == VOLLE CE-DefaultList.
    static_assert(std::is_same_v<TC01_S2mix::StaticAxisVariants, ce01::AllPageTypes>);
    static_assert(std::is_same_v<TC14_S2mix::StaticAxisVariants, ce14::AllHandles>);
    // (b) belegte Achsen bleiben auf je 1 Prüfling-Variante reduziert.
    static_assert(mp11::mp_size<TC07_S2mix::StaticAxisVariants>::value == 1);
    static_assert(mp11::mp_size<TC11_S2mix::StaticAxisVariants>::value == 1);

    using EngineS2mix = pe::PermutationEngine<TC07_S2mix, TC01_S2mix, TC14_S2mix, TC11_S2mix>;
    constexpr std::size_t n01 = mp11::mp_size<ce01::AllPageTypes>::value;
    constexpr std::size_t n14 = mp11::mp_size<ce14::AllHandles>::value;

    // (c) Stufe-2-Raum des Prototyps = Produkt NUR über die leeren Achsen (01,14) — NICHT die "1" der Teilsicht.
    static_assert(EngineS2mix::count() == n01 * n14);
    static_assert(EngineS2mix::count() > 1);  // ⇒ die leeren Achsen expandieren echt aus
    static_assert(n01 >= 2 && n14 >= 2);      // beide CE-Achsen haben mehrere Default-Algorithmen
    SUCCEED();
}

// =================================================================
// V41.F.6.1 F.5-Schritt — prt-art-Slots komponieren in codegen-fähige volle Anatomie
//
// Brücke: die gemergten Slots bilden eine vollständige 17-Achsen-Composition, die das
// anatomy_codegen_tool in eine DLL materialisieren kann (IsComposition + HasCompositionLocation).
// =================================================================

namespace cea = ::comdare::cache_engine::anatomy;

TEST(F5_PrtArtComposition, IsCodegenEligibleComposition) {
    using C = ::comdare::prt_art::slots::PrtArtCompositionDemo;
    // 17-Achsen-Vollständigkeit (das prüft das Codegen-Tool via descriptor_from_composition).
    static_assert(cea::IsComposition<C>);
    static_assert(cea::HasCompositionLocation<C>);
    static_assert(cea::composition_organ_count<C>::value == 19);   // 17 SA-Achsen + queuing q1/q2 (Doc 30 §8.0)
    // Codegen-Lokalisierung korrekt gesetzt.
    EXPECT_EQ(C::name, std::string_view{"PrtArtCompositionDemo"});
    EXPECT_EQ(C::cpp_type_name, std::string_view{"::comdare::prt_art::slots::PrtArtCompositionDemo"});
    EXPECT_FALSE(std::string_view{C::header_include}.empty());
}

TEST(F5_PrtArtComposition, PrtArtSlotsAreTheOverriddenAxes) {
    using C = ::comdare::prt_art::slots::PrtArtCompositionDemo;
    // prefetch + value_handle sind prt-art-Slot-Wrapper; der Rest CE-Default (ArtComposition).
    static_assert(std::is_same_v<C::prefetch,     ::comdare::prt_art::slots::axis_07::PrtArtRedirectPrefetch>);
    static_assert(std::is_same_v<C::value_handle, ::comdare::prt_art::slots::axis_14::PrtArtChainRefHandle>);
    static_assert(std::is_same_v<C::search_algo,  C::Base::search_algo>);   // CE-Default unverändert
    static_assert(std::is_same_v<C::telemetry,    C::Base::telemetry>);     // CE LeafOnly (nicht PerNode-Anti-Pattern)
    SUCCEED();
}

TEST(F5_PrtArtComposition, FullAnatomyInstantiatesFromPrtArtSlots) {
    // Die volle SearchAlgorithmAnatomy baut aus der prt-art-Slot-Composition — die Codegen-Einheit.
    using Anatomy = cea::SearchAlgorithmAnatomy<::comdare::prt_art::slots::PrtArtCompositionDemo>;
    Anatomy anatomy{};   // default-konstruierbar → instanziierbar als Codegen-Ziel
    (void)anatomy;
    SUCCEED();
}
