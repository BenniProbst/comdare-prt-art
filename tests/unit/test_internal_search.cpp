// Tests fuer 4 internal search types (Density-Schwellwerte 25/50/75% — REV 6 §5.17)

#include <prt_art/internal_search/array_256.hpp>
#include <prt_art/internal_search/array_65535.hpp>
#include <prt_art/internal_search/vector_u8_u8.hpp>
#include <prt_art/internal_search/vector_u16_u16.hpp>

#include <gtest/gtest.h>

namespace is = comdare::prt_art::internal_search;

// ─────────────────────────────────────────────────────────────────────────────
// Array<256> — Density bis 25%
// ─────────────────────────────────────────────────────────────────────────────

TEST(Array256, CapacityAndDensityThreshold) {
    EXPECT_EQ(is::Array256::kCapacity, 256u);
    EXPECT_DOUBLE_EQ(is::Array256::kDensityMaxPercent, 25.0);
}

TEST(Array256, EmptyLookupReturnsNullopt) {
    is::Array256 a;
    EXPECT_FALSE(a.lookup(42).has_value());
    EXPECT_EQ(a.occupied_count(), 0u);
}

TEST(Array256, InsertAndLookup) {
    is::Array256 a;
    a.insert(42, 1234);
    a.insert(100, 5678);
    auto v = a.lookup(42);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 1234u);
    EXPECT_EQ(*a.lookup(100), 5678u);
    EXPECT_EQ(a.occupied_count(), 2u);
}

TEST(Array256, EraseRemovesEntry) {
    is::Array256 a;
    a.insert(7, 99);
    a.erase(7);
    EXPECT_FALSE(a.lookup(7).has_value());
    EXPECT_EQ(a.occupied_count(), 0u);
}

TEST(Array256, DensityCalculation) {
    is::Array256 a;
    for (std::uint16_t i = 0; i < 64; ++i) a.insert(static_cast<std::uint8_t>(i), i);
    EXPECT_DOUBLE_EQ(a.density_percent(), 25.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Array<65535> — Density 25-50%
// ─────────────────────────────────────────────────────────────────────────────

TEST(Array65535, CapacityAndDensityThresholds) {
    EXPECT_EQ(is::Array65535::kCapacity,
              65536u); // M-PA-02-Fix: uint16 hat 65536 Werte (0..65535); vorher OOB bei insert(65535)
    EXPECT_DOUBLE_EQ(is::Array65535::kDensityMinPercent, 25.0);
    EXPECT_DOUBLE_EQ(is::Array65535::kDensityMaxPercent, 50.0);
}

TEST(Array65535, InsertAndLookup) {
    is::Array65535 a;
    a.insert(50000, 999);
    auto v = a.lookup(50000);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 999u);
}

TEST(Array65535, MaxDiscriminator65535InBounds) {
    // M-PA-02-Regression: der uint16-Maximalwert 65535 muss insertier-/lookup-bar sein (war OOB bei kCapacity=65535).
    is::Array65535 a;
    a.insert(65535, 4242u);
    EXPECT_EQ(a.lookup(65535), std::optional<std::uint64_t>{4242u});
    a.erase(65535);
    EXPECT_EQ(a.lookup(65535), std::nullopt);
}

TEST(Array65535, EmptyLookupReturnsNullopt) {
    is::Array65535 a;
    EXPECT_FALSE(a.lookup(0).has_value());
    EXPECT_FALSE(a.lookup(65534).has_value());
}

// ─────────────────────────────────────────────────────────────────────────────
// Vector<u8,u8> — Density 50-75%
// ─────────────────────────────────────────────────────────────────────────────

TEST(VectorU8U8, CapacityAndDensityThresholds) {
    EXPECT_EQ(is::VectorU8U8::kCapacity, 256u);
    EXPECT_DOUBLE_EQ(is::VectorU8U8::kDensityMinPercent, 50.0);
    EXPECT_DOUBLE_EQ(is::VectorU8U8::kDensityMaxPercent, 75.0);
}

TEST(VectorU8U8, InsertSortedByDiscriminator) {
    is::VectorU8U8 v;
    v.insert(10, 100);
    v.insert(5, 50);
    v.insert(20, 200);
    auto const& entries = v.entries();
    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries[0].discriminator, 5);
    EXPECT_EQ(entries[1].discriminator, 10);
    EXPECT_EQ(entries[2].discriminator, 20);
}

TEST(VectorU8U8, LookupViaBinarySearch) {
    is::VectorU8U8 v;
    for (std::uint8_t d : {200, 50, 5, 100, 150}) v.insert(d, d * 10u);
    EXPECT_EQ(*v.lookup(50), 500u);
    EXPECT_EQ(*v.lookup(150), 1500u);
    EXPECT_FALSE(v.lookup(99).has_value());
}

TEST(VectorU8U8, EraseRemovesEntry) {
    is::VectorU8U8 v;
    v.insert(7, 70);
    v.insert(8, 80);
    v.erase(7);
    EXPECT_EQ(v.occupied_count(), 1u);
    EXPECT_FALSE(v.lookup(7).has_value());
    EXPECT_TRUE(v.lookup(8).has_value());
}

// ─────────────────────────────────────────────────────────────────────────────
// Vector<u16,u16> — Density >75%
// ─────────────────────────────────────────────────────────────────────────────

TEST(VectorU16U16, CapacityAndDensityThreshold) {
    EXPECT_EQ(is::VectorU16U16::kCapacity, 65535u);
    EXPECT_DOUBLE_EQ(is::VectorU16U16::kDensityMinPercent, 75.0);
}

TEST(VectorU16U16, InsertAndLookup) {
    is::VectorU16U16 v;
    v.insert(40000, 1);
    v.insert(10000, 2);
    v.insert(60000, 3);
    EXPECT_EQ(*v.lookup(10000), 2u);
    EXPECT_EQ(*v.lookup(40000), 1u);
    EXPECT_EQ(*v.lookup(60000), 3u);
    EXPECT_FALSE(v.lookup(50000).has_value());
}

TEST(VectorU16U16, EntriesSortedByDiscriminator) {
    is::VectorU16U16 v;
    for (std::uint16_t d : {30000, 10000, 50000, 20000}) v.insert(d, d);
    auto const& entries = v.entries();
    EXPECT_LT(entries[0].discriminator, entries[1].discriminator);
    EXPECT_LT(entries[1].discriminator, entries[2].discriminator);
    EXPECT_LT(entries[2].discriminator, entries[3].discriminator);
}

// ─────────────────────────────────────────────────────────────────────────────
// Density-Schwellwerte 25/50/75 — Konsistenz aller 4 Typen
// ─────────────────────────────────────────────────────────────────────────────

TEST(InternalSearch, DensityThresholdsFormPartition) {
    EXPECT_DOUBLE_EQ(is::Array256::kDensityMaxPercent, is::Array65535::kDensityMinPercent);
    EXPECT_DOUBLE_EQ(is::Array65535::kDensityMaxPercent, is::VectorU8U8::kDensityMinPercent);
    EXPECT_DOUBLE_EQ(is::VectorU8U8::kDensityMaxPercent, is::VectorU16U16::kDensityMinPercent);
}
