// V41.E11 Phase B — prt-art Pruefling-Registrierung Integration-Test.
// Wird NUR im Plugin-Controller-Build gebaut (cache-engine mit COMDARE_CE_PRUEFLINGE=<prt-art>),
// via comdare_pruefling.cmake (das comdare_add_test aufruft). Verifiziert end-to-end, dass
// die cache-engine prt-art als Plugin laedt + dessen Factory registriert + Prueflinge erzeugt.

#include <gtest/gtest.h>

#include <prt_art/identity/prt_art_pruefling_factory.hpp>
#include <cache_engine/api/pruefling_registry.hpp>

#include <string_view>

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
