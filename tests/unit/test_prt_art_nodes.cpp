// Tests fuer 2 PRT-ART Node-Typen (REV 6 §5.17)

#include <prt_art/nodes/redirect_node.hpp>
#include <prt_art/nodes/bplus_node.hpp>

#include <gtest/gtest.h>

namespace nodes = comdare::prt_art::nodes;

// ─────────────────────────────────────────────────────────────────────────────
// RedirectNode
// ─────────────────────────────────────────────────────────────────────────────

TEST(RedirectNode, PriorityIsP0) {
    EXPECT_EQ(nodes::RedirectNode::kPriorityP0, 0);
}

TEST(RedirectNode, StoresRestSuffix) {
    std::vector<std::byte> suffix = {std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};
    nodes::RedirectNode n{suffix, 42};
    EXPECT_EQ(n.bytes_consumed(), 3u);
    EXPECT_EQ(n.terminal_handle(), 42u);
}

TEST(RedirectNode, FullMatchReturnsAllBytes) {
    std::vector<std::byte> suffix = {std::byte{'x'}, std::byte{'y'}};
    nodes::RedirectNode n{suffix};
    std::vector<std::byte> key = {std::byte{'x'}, std::byte{'y'}};
    EXPECT_EQ(n.try_match(key), 2u);
    EXPECT_TRUE(n.is_complete_match(key));
}

TEST(RedirectNode, PartialMatchReturnsMatchedPrefixLen) {
    std::vector<std::byte> suffix = {std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};
    nodes::RedirectNode n{suffix};
    std::vector<std::byte> key = {std::byte{'a'}, std::byte{'b'}, std::byte{'X'}};
    EXPECT_EQ(n.try_match(key), 2u);
    EXPECT_FALSE(n.is_complete_match(key));
}

TEST(RedirectNode, ShortKeyReturnsLengthOfKey) {
    std::vector<std::byte> suffix = {std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};
    nodes::RedirectNode n{suffix};
    std::vector<std::byte> key = {std::byte{'a'}, std::byte{'b'}};
    EXPECT_EQ(n.try_match(key), 2u);
    EXPECT_FALSE(n.is_complete_match(key));
}

// ─────────────────────────────────────────────────────────────────────────────
// BPlusNode
// ─────────────────────────────────────────────────────────────────────────────

TEST(BPlusNode, PriorityIsP0) {
    EXPECT_EQ(nodes::BPlusNode::kPriorityP0, 0);
}

TEST(BPlusNode, DefaultKindIsArray256) {
    nodes::BPlusNode n;
    EXPECT_EQ(n.kind(), nodes::InternalSearchKind::Array256);
    EXPECT_EQ(nodes::BPlusNode::capacity_for(n.kind()), 256u);
}

TEST(BPlusNode, InsertSlotIncrementsKeyCount) {
    nodes::BPlusNode n;
    n.insert_slot(5, 100);
    n.insert_slot(7, 200);
    n.insert_slot(9, 300);
    EXPECT_EQ(n.key_count(), 3u);
    EXPECT_EQ(n.slots().size(), 3u);
}

TEST(BPlusNode, DensityCalculationMatchesCapacity) {
    nodes::BPlusNode n{nodes::InternalSearchKind::Array256};
    for (std::uint8_t i = 0; i < 64; ++i) n.insert_slot(i, i);
    EXPECT_DOUBLE_EQ(n.density_percent(), 25.0);   // 64/256 = 25%
}

TEST(BPlusNode, RecommendedKindFollowsThresholds) {
    // Density-Schwellwerte 25/50/75
    nodes::BPlusNode n{nodes::InternalSearchKind::Array256};
    EXPECT_EQ(n.recommended_kind(), nodes::InternalSearchKind::Array256);  // 0%

    for (std::uint8_t i = 0; i < 60; ++i) n.insert_slot(i, i);  // 23%
    EXPECT_EQ(n.recommended_kind(), nodes::InternalSearchKind::Array256);

    for (std::uint8_t i = 60; i < 70; ++i) n.insert_slot(i, i);  // 27%
    EXPECT_EQ(n.recommended_kind(), nodes::InternalSearchKind::Array65535);

    for (std::uint8_t i = 70; i < 200; ++i) n.insert_slot(i, i);  // 78%
    EXPECT_EQ(n.recommended_kind(), nodes::InternalSearchKind::VectorU16U16);
}

TEST(BPlusNode, CapacityForKindMatchesSpec) {
    EXPECT_EQ(nodes::BPlusNode::capacity_for(nodes::InternalSearchKind::Array256),     256u);
    EXPECT_EQ(nodes::BPlusNode::capacity_for(nodes::InternalSearchKind::Array65535), 65535u);
    EXPECT_EQ(nodes::BPlusNode::capacity_for(nodes::InternalSearchKind::VectorU8U8),   256u);
    EXPECT_EQ(nodes::BPlusNode::capacity_for(nodes::InternalSearchKind::VectorU16U16),65535u);
}

TEST(BPlusNode, FourInternalSearchKindsAreDistinct) {
    EXPECT_EQ(static_cast<int>(nodes::InternalSearchKind::Array256),     0);
    EXPECT_EQ(static_cast<int>(nodes::InternalSearchKind::Array65535),   1);
    EXPECT_EQ(static_cast<int>(nodes::InternalSearchKind::VectorU8U8),   2);
    EXPECT_EQ(static_cast<int>(nodes::InternalSearchKind::VectorU16U16), 3);
}
