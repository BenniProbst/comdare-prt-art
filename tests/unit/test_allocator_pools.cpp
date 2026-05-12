// Tests fuer 4+2 Allocator-Pools (REV 6 §5.17)
// Pool-Namen: A_TrieHuelle / B_DensePages / C_MultiLevel / D_DecisionSpan / R_Rest
//             V_StaticValue / V_DynamicValue

#include <prt_art/allocator/pool_descriptor.hpp>
#include <prt_art/allocator/pool_router.hpp>
#include <prt_art/allocator/pool_set.hpp>

#include <gtest/gtest.h>

namespace al = comdare::prt_art::allocator;

// ─────────────────────────────────────────────────────────────────────────────
// PoolDescriptor + PoolStatistics
// ─────────────────────────────────────────────────────────────────────────────

TEST(PoolDescriptor, KindNamesAreSpecific) {
    EXPECT_EQ(al::name(al::PoolKind::A_TrieHuelle),    "A_TrieHuelle");
    EXPECT_EQ(al::name(al::PoolKind::B_DensePages),    "B_DensePages");
    EXPECT_EQ(al::name(al::PoolKind::C_MultiLevel),    "C_MultiLevel");
    EXPECT_EQ(al::name(al::PoolKind::D_DecisionSpan),  "D_DecisionSpan");
    EXPECT_EQ(al::name(al::PoolKind::R_Rest),          "R_Rest");
    EXPECT_EQ(al::name(al::PoolKind::V_StaticValue),   "V_StaticValue");
    EXPECT_EQ(al::name(al::PoolKind::V_DynamicValue),  "V_DynamicValue");
}

TEST(PoolStatistics, OnAllocIncrementsCounters) {
    al::PoolStatistics s;
    s.on_alloc(64);
    s.on_alloc(128);
    EXPECT_EQ(s.allocations, 2u);
    EXPECT_EQ(s.bytes_allocated, 192u);
    EXPECT_EQ(s.current_bytes_in_use, 192u);
    EXPECT_EQ(s.peak_bytes_in_use, 192u);
}

TEST(PoolStatistics, OnDeallocReducesInUse) {
    al::PoolStatistics s;
    s.on_alloc(100);
    s.on_alloc(50);                 // peak 150, in-use 150
    s.on_dealloc(50);               // in-use 100, peak bleibt 150
    EXPECT_EQ(s.deallocations, 1u);
    EXPECT_EQ(s.bytes_deallocated, 50u);
    EXPECT_EQ(s.current_bytes_in_use, 100u);
    EXPECT_EQ(s.peak_bytes_in_use, 150u);
}

TEST(PoolStatistics, OnDeallocClampsAtZero) {
    al::PoolStatistics s;
    s.on_alloc(10);
    s.on_dealloc(50);   // mehr deallociert als alloziert — clamp at 0
    EXPECT_EQ(s.current_bytes_in_use, 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// PoolSet
// ─────────────────────────────────────────────────────────────────────────────

TEST(PoolSet, SevenPoolsExist) {
    al::PoolSet ps;
    EXPECT_EQ(al::PoolSet::kPoolCount, 7u);
    EXPECT_EQ(ps.all().size(), 7u);
}

TEST(PoolSet, EachPoolKindIsAccessible) {
    al::PoolSet ps;
    EXPECT_EQ(ps.get(al::PoolKind::A_TrieHuelle).kind,    al::PoolKind::A_TrieHuelle);
    EXPECT_EQ(ps.get(al::PoolKind::V_DynamicValue).kind,  al::PoolKind::V_DynamicValue);
}

TEST(PoolSet, ConfigureSetsSlotSizeAndCapacity) {
    al::PoolSet ps;
    ps.configure(al::PoolKind::B_DensePages, 256, 4096);
    auto const& d = ps.get(al::PoolKind::B_DensePages);
    EXPECT_EQ(d.slot_size_bytes, 256u);
    EXPECT_EQ(d.capacity_slots, 4096u);
}

TEST(PoolSet, NoteAllocAggregatesAcrossPools) {
    al::PoolSet ps;
    ps.note_alloc(al::PoolKind::A_TrieHuelle,   64);
    ps.note_alloc(al::PoolKind::B_DensePages,   128);
    ps.note_alloc(al::PoolKind::V_StaticValue,  8);
    EXPECT_EQ(ps.total_allocated_bytes(), 200u);
    EXPECT_EQ(ps.total_in_use_bytes(),     200u);
}

TEST(PoolSet, NoteDeallocReducesInUse) {
    al::PoolSet ps;
    ps.note_alloc(al::PoolKind::B_DensePages, 100);
    ps.note_dealloc(al::PoolKind::B_DensePages, 100);
    EXPECT_EQ(ps.total_in_use_bytes(), 0u);
    EXPECT_EQ(ps.get(al::PoolKind::B_DensePages).stats.bytes_allocated, 100u);
}

// ─────────────────────────────────────────────────────────────────────────────
// PoolRouter
// ─────────────────────────────────────────────────────────────────────────────

TEST(PoolRouter, RoutePageEncodingsToCorrectPools) {
    EXPECT_EQ(al::PoolRouter::route_page(al::PageEncodingTag::Redirect),        al::PoolKind::A_TrieHuelle);
    EXPECT_EQ(al::PoolRouter::route_page(al::PageEncodingTag::DenseByte),       al::PoolKind::B_DensePages);
    EXPECT_EQ(al::PoolRouter::route_page(al::PageEncodingTag::SparsePatricia),  al::PoolKind::B_DensePages);
    EXPECT_EQ(al::PoolRouter::route_page(al::PageEncodingTag::MultilevelDense), al::PoolKind::C_MultiLevel);
    EXPECT_EQ(al::PoolRouter::route_page(al::PageEncodingTag::DecisionSpan),    al::PoolKind::D_DecisionSpan);
    EXPECT_EQ(al::PoolRouter::route_page(al::PageEncodingTag::CustomAligned),   al::PoolKind::D_DecisionSpan);
}

TEST(PoolRouter, RouteValueHandlesToValuePools) {
    EXPECT_EQ(al::PoolRouter::route_value(al::ValueHandleTag::Inline),   al::PoolKind::V_StaticValue);
    EXPECT_EQ(al::PoolRouter::route_value(al::ValueHandleTag::External), al::PoolKind::V_DynamicValue);
    EXPECT_EQ(al::PoolRouter::route_value(al::ValueHandleTag::ChainRef), al::PoolKind::V_DynamicValue);
}

TEST(PoolRouter, MiscRoutesToRestPool) {
    EXPECT_EQ(al::PoolRouter::route_misc(), al::PoolKind::R_Rest);
}
