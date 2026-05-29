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
