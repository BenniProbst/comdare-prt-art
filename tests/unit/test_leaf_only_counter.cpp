// V34.E.2 (2026-05-21) - LeafOnlyCounter + PerNodeCounter Tests
//
// @achse 11
// @subsystem PA

#include "prt_art/telemetry/leaf_only_counter.hpp"

#include <gtest/gtest.h>

#include <thread>
#include <vector>

namespace tel = comdare::prt_art::telemetry;

TEST(LeafOnlyCounter, InnerNodeAccessIsNoOp) {
    tel::LeafOnlyCounter<std::uint64_t> counter;
    counter.record_access(0x100, /*is_leaf=*/false);
    counter.record_access(0x100, /*is_leaf=*/false);
    counter.record_access(0x100, /*is_leaf=*/false);
    EXPECT_EQ(counter.access_count(0x100), 0u);
    EXPECT_EQ(counter.tracked_leaves(), 0u);
}

TEST(LeafOnlyCounter, LeafAccessIncrements) {
    tel::LeafOnlyCounter<std::uint64_t> counter;
    counter.record_access(0x200, /*is_leaf=*/true);
    counter.record_access(0x200, /*is_leaf=*/true);
    EXPECT_EQ(counter.access_count(0x200), 2u);
    EXPECT_EQ(counter.tracked_leaves(), 1u);
}

TEST(LeafOnlyCounter, MixedLeafAndInnerNodes) {
    tel::LeafOnlyCounter<std::uint64_t> counter;
    counter.record_access(0x100, /*is_leaf=*/false);  // inner
    counter.record_access(0x200, /*is_leaf=*/true);   // leaf
    counter.record_access(0x300, /*is_leaf=*/true);   // leaf
    counter.record_access(0x300, /*is_leaf=*/true);   // leaf again
    EXPECT_EQ(counter.access_count(0x100), 0u);
    EXPECT_EQ(counter.access_count(0x200), 1u);
    EXPECT_EQ(counter.access_count(0x300), 2u);
    EXPECT_EQ(counter.tracked_leaves(), 2u);
    EXPECT_EQ(counter.total_accesses(), 3u);
}

TEST(LeafOnlyCounter, CachePressureEstimate) {
    tel::LeafOnlyCounter<std::uint64_t> counter;
    // 3 leaves tracked, alle mit count > 0 -> pressure = 1.0
    counter.record_access(0x100, true);
    counter.record_access(0x200, true);
    counter.record_access(0x300, true);
    EXPECT_DOUBLE_EQ(counter.cache_pressure_estimate(), 1.0);
}

TEST(LeafOnlyCounter, EmptyCounterPressureZero) {
    tel::LeafOnlyCounter<std::uint64_t> counter;
    EXPECT_DOUBLE_EQ(counter.cache_pressure_estimate(), 0.0);
}

TEST(LeafOnlyCounter, ConcurrentAccessIsThreadSafeOnSameLeaf) {
    tel::LeafOnlyCounter<std::uint64_t> counter;
    constexpr int kThreads = 4;
    constexpr int kPerThread = 1000;
    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&counter]() {
            for (int i = 0; i < kPerThread; ++i) {
                counter.record_access(0x42, /*is_leaf=*/true);
            }
        });
    }
    for (auto& th : threads) th.join();
    EXPECT_EQ(counter.access_count(0x42), kThreads * kPerThread);
}

TEST(LeafOnlyCounter, Reset) {
    tel::LeafOnlyCounter<std::uint64_t> counter;
    counter.record_access(0x100, true);
    counter.record_access(0x200, true);
    EXPECT_EQ(counter.tracked_leaves(), 2u);
    counter.reset();
    EXPECT_EQ(counter.tracked_leaves(), 0u);
    EXPECT_EQ(counter.access_count(0x100), 0u);
}

// ============================================================================
// ANTI-PATTERN PerNodeCounter Tests (Vergleichs-Validierung)
// ============================================================================

TEST(PerNodeCounter, IncrementsAllNodesInclInner) {
    tel::PerNodeCounter<std::uint64_t> counter;
    counter.record_access(0x100, /*is_leaf=*/false);  // inner BUT counts
    counter.record_access(0x200, /*is_leaf=*/true);
    EXPECT_EQ(counter.access_count(0x100), 1u);
    EXPECT_EQ(counter.access_count(0x200), 1u);
    EXPECT_EQ(counter.tracked_nodes(), 2u);
}

TEST(PerNodeCounter, TrackedNodesGrowsLinearlyWithUniqueAccesses) {
    tel::PerNodeCounter<std::uint64_t> counter;
    for (std::uint64_t i = 0; i < 100; ++i) {
        counter.record_access(i, /*is_leaf=*/i % 3 == 0);
    }
    EXPECT_EQ(counter.tracked_nodes(), 100u);
    EXPECT_EQ(counter.total_accesses(), 100u);
}

// ============================================================================
// Cross-Variant-Vergleich (validiert Kuehn-Behauptung dass LeafOnly <= PerNode)
// ============================================================================

TEST(KuehnComparison, LeafOnlyTracksFewerNodesThanPerNode) {
    tel::LeafOnlyCounter<std::uint64_t> leaf_only;
    tel::PerNodeCounter<std::uint64_t> per_node;

    // Simuliere Baum-Traversierung: 1 leaf-Hit benoetigt 3 Inner-Node-Hits (typische Tiefe)
    for (std::uint64_t leaf = 1000; leaf < 1100; ++leaf) {
        // Traversal: inner_root -> inner_l1 -> inner_l2 -> leaf
        leaf_only.record_access(0x1, false);   // root
        leaf_only.record_access(0x2, false);   // L1
        leaf_only.record_access(0x3, false);   // L2
        leaf_only.record_access(leaf, true);   // leaf

        per_node.record_access(0x1, false);
        per_node.record_access(0x2, false);
        per_node.record_access(0x3, false);
        per_node.record_access(leaf, true);
    }

    // LeafOnly traked nur die 100 Leaves
    EXPECT_EQ(leaf_only.tracked_leaves(), 100u);
    EXPECT_EQ(leaf_only.total_accesses(), 100u);

    // PerNode traked 3 inner + 100 leaves = 103
    EXPECT_EQ(per_node.tracked_nodes(), 103u);
    EXPECT_EQ(per_node.total_accesses(), 400u);  // 100 * 4

    // KRITISCH: LeafOnly hat 4x weniger Writes -> 4x weniger Cache-Line-Ping-Pong
    EXPECT_LT(leaf_only.total_accesses(), per_node.total_accesses());
}
