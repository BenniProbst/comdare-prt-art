// Tests fuer PRT-ART Mess-Hooks (DensityTracker + H1/H2/H3-Metriken) — REV 6 §5.17

#include <prt_art/measurement/density_tracker.hpp>
#include <prt_art/measurement/hypothesis_metrics.hpp>

#include <gtest/gtest.h>

namespace meas = comdare::prt_art::measurement;

// ─────────────────────────────────────────────────────────────────────────────
// DensityTracker
// ─────────────────────────────────────────────────────────────────────────────

TEST(DensityTracker, EmptyAfterConstruction) {
    meas::DensityTracker t;
    EXPECT_EQ(t.total_observations(), 0u);
    EXPECT_EQ(t.tracked_node_count(), 0u);
    EXPECT_DOUBLE_EQ(t.density_for(42), 0.0);
}

TEST(DensityTracker, RecordAndQuery) {
    meas::DensityTracker t;
    t.record(1, 25.5);
    t.record(2, 67.0);
    t.record(3, 92.0);
    EXPECT_EQ(t.total_observations(), 3u);
    EXPECT_EQ(t.tracked_node_count(), 3u);
    EXPECT_DOUBLE_EQ(t.density_for(2), 67.0);
}

TEST(DensityTracker, Histogram4BucketsByThresholds) {
    meas::DensityTracker t;
    t.record(1, 10.0); // bucket 0 (<25)
    t.record(2, 20.0); // bucket 0
    t.record(3, 30.0); // bucket 1 (25-50)
    t.record(4, 60.0); // bucket 2 (50-75)
    t.record(5, 80.0); // bucket 3 (>=75)
    t.record(6, 95.0); // bucket 3
    auto h = t.histogram();
    ASSERT_EQ(h.size(), 4u);
    EXPECT_EQ(h[0].node_count, 2u);
    EXPECT_EQ(h[1].node_count, 1u);
    EXPECT_EQ(h[2].node_count, 1u);
    EXPECT_EQ(h[3].node_count, 2u);
    EXPECT_DOUBLE_EQ(h[3].avg_density_percent(), 87.5);
}

TEST(DensityTracker, ResetClearsHistory) {
    meas::DensityTracker t;
    t.record(1, 50.0);
    t.reset();
    EXPECT_EQ(t.total_observations(), 0u);
    EXPECT_EQ(t.tracked_node_count(), 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// H1 — Page-Type-Cost
// ─────────────────────────────────────────────────────────────────────────────

TEST(H1PageTypeCost, NoteLookupBuildsOnlineMean) {
    meas::H1PageTypeCost h1;
    h1.note_lookup("Redirect", 100.0);
    h1.note_lookup("Redirect", 200.0);
    h1.note_lookup("Redirect", 300.0);
    EXPECT_DOUBLE_EQ(h1.mean_cycles_per_lookup_by_page_type["Redirect"], 200.0);
    EXPECT_EQ(h1.sample_count_by_page_type["Redirect"], 3u);
}

TEST(H1PageTypeCost, MultiplePageTypesTrackedSeparately) {
    meas::H1PageTypeCost h1;
    h1.note_lookup("DenseByte", 50.0);
    h1.note_lookup("SparsePatricia", 150.0);
    EXPECT_DOUBLE_EQ(h1.mean_cycles_per_lookup_by_page_type["DenseByte"], 50.0);
    EXPECT_DOUBLE_EQ(h1.mean_cycles_per_lookup_by_page_type["SparsePatricia"], 150.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// H2 — Code-Quality
// ─────────────────────────────────────────────────────────────────────────────

TEST(H2CodeQuality, ScoreAndNote) {
    meas::H2CodeQuality h2;
    h2.score("P01_unodb", 0.92, "well-structured, BSD-licensed");
    h2.score("P02_hot", 0.85);
    EXPECT_DOUBLE_EQ(h2.quality_score_by_source["P01_unodb"], 0.92);
    EXPECT_EQ(h2.note_by_source["P01_unodb"], "well-structured, BSD-licensed");
    EXPECT_TRUE(h2.note_by_source.find("P02_hot") == h2.note_by_source.end());
}

// ─────────────────────────────────────────────────────────────────────────────
// H3 — Inline-vs-External-Verteilung
// ─────────────────────────────────────────────────────────────────────────────

TEST(H3InlineVsExternal, EmptyTotalIsZero) {
    meas::H3InlineVsExternalDistribution h3;
    EXPECT_EQ(h3.total(), 0u);
    EXPECT_DOUBLE_EQ(h3.inline_share(), 0.0);
}

TEST(H3InlineVsExternal, SharesSumToOne) {
    meas::H3InlineVsExternalDistribution h3;
    for (int i = 0; i < 60; ++i) h3.note_inline();
    for (int i = 0; i < 30; ++i) h3.note_external();
    for (int i = 0; i < 10; ++i) h3.note_chain();
    EXPECT_EQ(h3.total(), 100u);
    EXPECT_DOUBLE_EQ(h3.inline_share(), 0.60);
    EXPECT_DOUBLE_EQ(h3.external_share(), 0.30);
    EXPECT_DOUBLE_EQ(h3.chain_share(), 0.10);
}

TEST(PrtArtHypothesisMetrics, AllThreeAccessible) {
    meas::PrtArtHypothesisMetrics m;
    m.h1.note_lookup("Redirect", 100.0);
    m.h2.score("P01", 0.9);
    m.h3.note_inline();
    EXPECT_EQ(m.h1.sample_count_by_page_type["Redirect"], 1u);
    EXPECT_DOUBLE_EQ(m.h2.quality_score_by_source["P01"], 0.9);
    EXPECT_EQ(m.h3.total(), 1u);
}
