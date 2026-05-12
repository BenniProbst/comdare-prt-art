// Tests fuer PRT-ART Prefetch-Strategien (REV 6 §5.17)

#include <prt_art/prefetch/distance_estimator.hpp>
#include <prt_art/prefetch/path_oriented_prefetch.hpp>
#include <prt_art/prefetch/redirect_prefetch.hpp>

#include <gtest/gtest.h>

namespace pf = comdare::prt_art::prefetch;

// ─────────────────────────────────────────────────────────────────────────────
// PrefetchDistanceEstimator
// ─────────────────────────────────────────────────────────────────────────────

TEST(DistanceEstimator, MinAndMaxBoundsAreEnforced) {
    EXPECT_EQ(pf::PrefetchDistanceEstimator::clamp(0),  pf::PrefetchDistanceEstimator::kMinDistance);
    EXPECT_EQ(pf::PrefetchDistanceEstimator::clamp(99), pf::PrefetchDistanceEstimator::kMaxDistance);
    EXPECT_EQ(pf::PrefetchDistanceEstimator::clamp(5),  5);
}

TEST(DistanceEstimator, SparseHighLatencyHasLargerDistance) {
    auto sparse = pf::PrefetchDistanceEstimator::estimate(10.0, 100.0);   // sehr sparse, DRAM-Latenz
    auto dense  = pf::PrefetchDistanceEstimator::estimate(95.0, 4.0);     // sehr dense, L1-Latenz
    EXPECT_GT(sparse, dense);
}

TEST(DistanceEstimator, DistanceIsAlwaysWithinBounds) {
    for (double d : {0.0, 25.0, 50.0, 75.0, 100.0}) {
        for (double l : {2.0, 10.0, 50.0, 200.0}) {
            auto e = pf::PrefetchDistanceEstimator::estimate(d, l);
            EXPECT_GE(e, pf::PrefetchDistanceEstimator::kMinDistance);
            EXPECT_LE(e, pf::PrefetchDistanceEstimator::kMaxDistance);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PathOrientedPrefetch
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathOrientedPrefetch, EmptyQueueAfterConstruction) {
    pf::PathOrientedPrefetch p;
    EXPECT_EQ(p.queue_depth(), 0u);
    EXPECT_EQ(p.total_enqueued(), 0u);
    EXPECT_EQ(p.suggest_next(), 0u);
}

TEST(PathOrientedPrefetch, EnqueueIncrementsCounters) {
    pf::PathOrientedPrefetch p;
    p.enqueue(100);
    p.enqueue(200);
    p.enqueue(300);
    EXPECT_EQ(p.total_enqueued(), 3u);
    EXPECT_EQ(p.queue_depth(), 3u);
}

TEST(PathOrientedPrefetch, SuggestExtrapolates) {
    pf::PathOrientedPrefetch p;
    p.enqueue(100);
    p.enqueue(200);
    EXPECT_EQ(p.suggest_next(), 300u);   // step = 100, extrapoliert auf 300
}

TEST(PathOrientedPrefetch, QueueIsBoundedAt16) {
    pf::PathOrientedPrefetch p;
    for (int i = 0; i < 100; ++i) p.enqueue(static_cast<std::uint64_t>(i));
    EXPECT_LE(p.queue_depth(), 16u);
    EXPECT_EQ(p.total_enqueued(), 100u);
}

TEST(PathOrientedPrefetch, ResetClearsQueue) {
    pf::PathOrientedPrefetch p;
    p.enqueue(1); p.enqueue(2);
    p.reset();
    EXPECT_EQ(p.queue_depth(), 0u);
    EXPECT_EQ(p.total_enqueued(), 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// RedirectPrefetch
// ─────────────────────────────────────────────────────────────────────────────

TEST(RedirectPrefetch, ScheduleTracksAddress) {
    pf::RedirectPrefetch r;
    r.schedule(0x1000);
    EXPECT_EQ(r.last_scheduled_addr(), 0x1000u);
    EXPECT_EQ(r.total_scheduled(), 1u);
}

TEST(RedirectPrefetch, FanOutSlotsRespectCacheLineSize) {
    pf::RedirectPrefetch r;
    r.schedule(0x1000);
    EXPECT_EQ(r.slot(0), 0x1000u);
    EXPECT_EQ(r.slot(1), 0x1000u + pf::RedirectPrefetch::kCacheLineSize);
    EXPECT_EQ(r.slot(2), 0x1000u - pf::RedirectPrefetch::kCacheLineSize);
}

TEST(RedirectPrefetch, NegativeOffsetIsClampedAtZero) {
    pf::RedirectPrefetch r;
    r.schedule(32);   // weniger als kCacheLineSize=64
    EXPECT_EQ(r.slot(2), 0u);
}

TEST(RedirectPrefetch, ResetClearsState) {
    pf::RedirectPrefetch r;
    r.schedule(0x500);
    r.reset();
    EXPECT_EQ(r.last_scheduled_addr(), 0u);
    EXPECT_EQ(r.total_scheduled(), 0u);
}
