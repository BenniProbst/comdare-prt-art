// V33.B.1 (2026-05-21) - Tests fuer DefaultLookupRegistry
//
// @subsystem PA
// @phase_owner PA

#include "prt_art/default_lookup/default_lookup_registry.hpp"

#include <gtest/gtest.h>
#include <string_view>

namespace dl = comdare::prt_art::default_lookup;

TEST(DefaultLookupRegistry, EnumeratesNineAxes) {
    constexpr auto axes = dl::DefaultLookupRegistry::enumerate();
    EXPECT_EQ(axes.size(), 9u);
    EXPECT_EQ(dl::DefaultLookupRegistry::count(), 9u);
}

TEST(DefaultLookupRegistry, KnownAxesAreFlagged) {
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("3.B"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("6.2"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("6.3"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("6.4"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("8.2"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("9"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("11"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("12"));
    EXPECT_TRUE(dl::DefaultLookupRegistry::is_default_lookup("13"));
}

TEST(DefaultLookupRegistry, PrtArtEigeneAchsenSindNichtDefaultLookup) {
    // PRT-ART hat eigene Implementierungen fuer 1, 2, 3.A, 3.M, 4, 5, 6.1, 6.5, 7, 8.1, 10, 14
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("1"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("2"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("3.A"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("3.M"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("4"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("5"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("6.1"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("6.5"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("7"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("8.1"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("10"));
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("14"));
}

TEST(DefaultLookupRegistry, UnknownAxisIsNotDefaultLookup) {
    EXPECT_FALSE(dl::DefaultLookupRegistry::is_default_lookup("99.99"));
}

TEST(DefaultLookupRegistry, EachAxisHasCeLibraryPath) {
    for (const auto& axis : dl::DefaultLookupRegistry::enumerate()) {
        EXPECT_FALSE(axis.ce_library_path.empty()) << "Axis " << axis.axis_id;
        EXPECT_FALSE(axis.indicator_header.empty()) << "Axis " << axis.axis_id;
        EXPECT_FALSE(axis.human_name.empty()) << "Axis " << axis.axis_id;
    }
}
