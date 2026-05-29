// V41.E11 Phase B — prt-art Pruefling-Registrierung Integration-Test.
// Wird NUR im Plugin-Controller-Build gebaut (cache-engine mit COMDARE_CE_PRUEFLINGE=<prt-art>),
// via comdare_pruefling.cmake (das comdare_add_test aufruft). Verifiziert end-to-end, dass
// die cache-engine prt-art als Plugin laedt + dessen Factory registriert + Prueflinge erzeugt.

#include <gtest/gtest.h>

#include <prt_art/identity/prt_art_pruefling_factory.hpp>
#include <cache_engine/api/pruefling_registry.hpp>

// V41.F.6.1 Phase B — Slot-Füllung-Demonstration (compile-time merge)
#include <prt_art/slots/axis_07_prefetch_slot.hpp>
#include <topics/prefetch/axis_07_prefetch/axis_07_prefetch_registry.hpp>
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
