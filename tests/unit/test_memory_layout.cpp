// Tests fuer Memory-Layout: TLB-Offset-Adressierung + Cache-Line-Aligned + Multi-Level
// REV 6 §5.17

#include <prt_art/memory_layout/byte_path.hpp>
#include <prt_art/memory_layout/cache_line_aligned_layout.hpp>
#include <prt_art/memory_layout/multi_level_layout.hpp>
#include <prt_art/memory_layout/virtual_offset_address.hpp>

#include <gtest/gtest.h>

#include <array>
#include <span>
#include <vector>

namespace ml = comdare::prt_art::memory_layout;

namespace {
// Helper: Bytes ueber expliziten span-Konstruktor — vermeidet MSVC array→span Issues
std::span<std::byte const> as_span(std::vector<std::byte> const& v) {
    return std::span<std::byte const>(v.data(), v.size());
}
}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// VirtualOffsetAddress — position = sum(k[i] * 256^(k_count-1-i))
// ─────────────────────────────────────────────────────────────────────────────

TEST(VirtualOffsetAddress, EmptyKeyReturnsZero) {
    EXPECT_EQ(ml::VirtualOffsetAddress::compute({}), 0u);
}

TEST(VirtualOffsetAddress, SingleByteIsValue) {
    std::vector<std::byte> k{std::byte{42}};
    EXPECT_EQ(ml::VirtualOffsetAddress::compute(as_span(k)), 42u);
}

TEST(VirtualOffsetAddress, TwoBytesA_B_IsAtimes256PlusB) {
    std::vector<std::byte> k{std::byte{3}, std::byte{7}};
    EXPECT_EQ(ml::VirtualOffsetAddress::compute(as_span(k)), 3u * 256u + 7u);
}

TEST(VirtualOffsetAddress, ThreeBytesFollowsHornerRule) {
    std::vector<std::byte> k{std::byte{1}, std::byte{2}, std::byte{3}};
    // pos = 1*65536 + 2*256 + 3 = 66051
    EXPECT_EQ(ml::VirtualOffsetAddress::compute(as_span(k)), 66051u);
}

TEST(VirtualOffsetAddress, CapacityFor3Is16M) {
    EXPECT_EQ(ml::VirtualOffsetAddress::capacity_for(3), 256u * 256u * 256u);
}

TEST(VirtualOffsetAddress, CapacityFor0Is0) {
    EXPECT_EQ(ml::VirtualOffsetAddress::capacity_for(0), 0u);
}

TEST(VirtualOffsetAddress, DecomposeIsInverse) {
    std::vector<std::byte> original{std::byte{0xAB}, std::byte{0xCD}, std::byte{0xEF}};
    auto pos = ml::VirtualOffsetAddress::compute(as_span(original));
    std::array<std::byte, 3> recovered{};
    ml::VirtualOffsetAddress::decompose(pos, 3, recovered.data());
    EXPECT_EQ(recovered[0], original[0]);
    EXPECT_EQ(recovered[1], original[1]);
    EXPECT_EQ(recovered[2], original[2]);
}

TEST(VirtualOffsetAddress, KeyLongerThanMaxIsClamped) {
    std::vector<std::byte> long_key;
    for (int i = 0; i < 16; ++i) long_key.push_back(static_cast<std::byte>(i + 1));
    auto pos_full = ml::VirtualOffsetAddress::compute(as_span(long_key));
    std::span<std::byte const> first8(long_key.data(), 8);
    EXPECT_EQ(pos_full, ml::VirtualOffsetAddress::compute(first8));
}

// ─────────────────────────────────────────────────────────────────────────────
// BytePath
// ─────────────────────────────────────────────────────────────────────────────

TEST(BytePath, DefaultIsExhausted) {
    ml::BytePath p;
    EXPECT_TRUE(p.exhausted());
    EXPECT_EQ(p.cursor(), 0u);
}

TEST(BytePath, ConsumeAdvancesCursor) {
    std::vector<std::byte> k{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}};
    ml::BytePath p{as_span(k)};
    auto first2 = p.consume(2);
    EXPECT_EQ(first2.size(), 2u);
    EXPECT_EQ(p.cursor(), 2u);
    EXPECT_EQ(p.remaining(), 3u);
}

TEST(BytePath, PeekDoesNotAdvance) {
    std::vector<std::byte> k{std::byte{1}, std::byte{2}, std::byte{3}};
    ml::BytePath p{as_span(k)};
    auto v = p.peek(2);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(p.cursor(), 0u);
}

TEST(BytePath, ConsumeMoreThanAvailableClamps) {
    std::vector<std::byte> k{std::byte{1}, std::byte{2}};
    ml::BytePath p{as_span(k)};
    auto v = p.consume(5);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(p.exhausted());
}

TEST(BytePath, ResetReturnsToStart) {
    std::vector<std::byte> k{std::byte{1}, std::byte{2}, std::byte{3}};
    ml::BytePath p{as_span(k)};
    p.advance(2);
    p.reset();
    EXPECT_EQ(p.cursor(), 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// CacheLineAlignedLayout
// ─────────────────────────────────────────────────────────────────────────────

TEST(CacheLineAlignedLayout, AlignedOffsetMultipleOf64) {
    EXPECT_EQ(ml::CacheLineAlignedLayout::aligned_offset(0), 0u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::aligned_offset(1), 64u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::aligned_offset(63), 64u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::aligned_offset(64), 64u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::aligned_offset(65), 128u);
}

TEST(CacheLineAlignedLayout, PaddingComputation) {
    EXPECT_EQ(ml::CacheLineAlignedLayout::padding_to_next_line(0), 0u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::padding_to_next_line(60), 4u);
}

TEST(CacheLineAlignedLayout, LinesNeeded) {
    EXPECT_EQ(ml::CacheLineAlignedLayout::lines_needed(0), 0u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::lines_needed(64), 1u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::lines_needed(65), 2u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::lines_needed(128), 2u);
    EXPECT_EQ(ml::CacheLineAlignedLayout::lines_needed(129), 3u);
}

TEST(CacheLineAlignedLayout, IsAlignedCheck) {
    EXPECT_TRUE(ml::CacheLineAlignedLayout::is_aligned(0));
    EXPECT_TRUE(ml::CacheLineAlignedLayout::is_aligned(64));
    EXPECT_TRUE(ml::CacheLineAlignedLayout::is_aligned(128));
    EXPECT_FALSE(ml::CacheLineAlignedLayout::is_aligned(1));
    EXPECT_FALSE(ml::CacheLineAlignedLayout::is_aligned(63));
}

TEST(CacheLineAlignedLayout, SlotViewReturnsCorrectSubspan) {
    std::vector<std::byte> buf(64 * 4, std::byte{0});
    auto bufv = std::span<std::byte>(buf.data(), buf.size());
    auto view0 = ml::CacheLineAlignedLayout::slot_view<32>(bufv, 0);
    auto view1 = ml::CacheLineAlignedLayout::slot_view<32>(bufv, 1);
    EXPECT_EQ(view0.size(), 32u);
    EXPECT_EQ(view1.size(), 32u);
    EXPECT_EQ(view1.data() - view0.data(), 64);   // jedes Slot padded auf 64 Byte
}

// ─────────────────────────────────────────────────────────────────────────────
// MultiLevelLayout
// ─────────────────────────────────────────────────────────────────────────────

TEST(MultiLevelLayout, DefaultBudgetsMatchExpectations) {
    ml::MultiLevelLayout layout;
    EXPECT_EQ(layout.budget().l1_bytes, 32u * 1024);
    EXPECT_EQ(layout.budget().l2_bytes, 1024u * 1024);
    EXPECT_EQ(layout.budget().l3_bytes, 32u * 1024 * 1024);
}

TEST(MultiLevelLayout, TierClassificationByOffset) {
    ml::MultiLevelLayout layout;
    EXPECT_EQ(layout.tier_for_offset(0),                   ml::CacheTier::L1Hot);
    EXPECT_EQ(layout.tier_for_offset(31 * 1024),           ml::CacheTier::L1Hot);
    EXPECT_EQ(layout.tier_for_offset(64 * 1024),           ml::CacheTier::L2Warm);
    EXPECT_EQ(layout.tier_for_offset(2 * 1024 * 1024),     ml::CacheTier::L3Cold);
    EXPECT_EQ(layout.tier_for_offset(64ull * 1024 * 1024), ml::CacheTier::Memory);
}

TEST(MultiLevelLayout, ResolveAddressUsesAlignedSlot) {
    ml::MultiLevelLayout layout;
    std::vector<std::byte> k{std::byte{1}};
    EXPECT_EQ(layout.resolve_address(as_span(k), 32), 64u);
}

TEST(MultiLevelLayout, KeyAtZeroIsHotTier) {
    ml::MultiLevelLayout layout;
    std::vector<std::byte> k{std::byte{0}};
    EXPECT_EQ(layout.tier_for_key(as_span(k), 32), ml::CacheTier::L1Hot);
}

TEST(MultiLevelLayout, BudgetCanBeChanged) {
    ml::MultiLevelLayout layout;
    ml::TierBudget custom{16 * 1024, 512 * 1024, 16 * 1024 * 1024};
    layout.set_budget(custom);
    EXPECT_EQ(layout.budget().l1_bytes, 16u * 1024);
}
