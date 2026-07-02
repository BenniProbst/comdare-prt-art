// Tests fuer ValueHandle (Inline / External / ChainRef + variant + Cost-Model)
// REV 6 §5.4

#include <prt_art/value_handle/chain_ref_handle.hpp>
#include <prt_art/value_handle/cost_model.hpp>
#include <prt_art/value_handle/external_handle.hpp>
#include <prt_art/value_handle/inline_handle.hpp>
#include <prt_art/value_handle/value_handle.hpp>

#include <gtest/gtest.h>

namespace vh = comdare::prt_art::value_handle;

// ─────────────────────────────────────────────────────────────────────────────
// InlineHandle
// ─────────────────────────────────────────────────────────────────────────────

TEST(InlineHandle, DefaultConstructorEmpty) {
    vh::InlineHandle<8> h;
    EXPECT_TRUE(h.empty());
    EXPECT_EQ(h.size(), 0u);
}

TEST(InlineHandle, StoreAndRead) {
    std::array<std::byte, 4> data{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
    vh::InlineHandle<8>      h{data};
    EXPECT_EQ(h.size(), 4u);
    auto bytes = h.bytes();
    ASSERT_EQ(bytes.size(), 4u);
    EXPECT_EQ(static_cast<int>(bytes[0]), 1);
    EXPECT_EQ(static_cast<int>(bytes[3]), 4);
}

TEST(InlineHandle, FitsCheck) {
    EXPECT_TRUE(vh::InlineHandle<8>::fits(8));
    EXPECT_FALSE(vh::InlineHandle<8>::fits(9));
    EXPECT_TRUE(vh::InlineHandle<16>::fits(16));
}

TEST(InlineHandle, OversizedStoreTruncates) {
    std::array<std::byte, 16> big{};
    for (int i = 0; i < 16; ++i) big[i] = static_cast<std::byte>(i);
    vh::InlineHandle<8> h{big};
    EXPECT_EQ(h.size(), 8u); // truncated
}

// ─────────────────────────────────────────────────────────────────────────────
// ExternalHandle
// ─────────────────────────────────────────────────────────────────────────────

TEST(ExternalHandle, DefaultIsInvalid) {
    vh::ExternalHandle e;
    EXPECT_FALSE(e.is_valid());
}

TEST(ExternalHandle, StoresOffsetAndSize) {
    vh::ExternalHandle e{1024, 256};
    EXPECT_TRUE(e.is_valid());
    EXPECT_EQ(e.pool_offset(), 1024u);
    EXPECT_EQ(e.value_bytes(), 256u);
}

TEST(ExternalHandle, InvalidateClears) {
    vh::ExternalHandle e{42, 8};
    e.invalidate();
    EXPECT_FALSE(e.is_valid());
}

// ─────────────────────────────────────────────────────────────────────────────
// ChainRefHandle
// ─────────────────────────────────────────────────────────────────────────────

TEST(ChainRefHandle, DefaultIsEmpty) {
    vh::ChainRefHandle c;
    EXPECT_TRUE(c.is_empty());
}

TEST(ChainRefHandle, ConstructWithChainHead) {
    vh::ChainRefHandle c{2048, 3};
    EXPECT_FALSE(c.is_empty());
    EXPECT_EQ(c.chain_head_offset(), 2048u);
    EXPECT_EQ(c.chain_length(), 3u);
}

TEST(ChainRefHandle, PrependIncrements) {
    vh::ChainRefHandle c{1, 1};
    c.prepend_node(42);
    EXPECT_EQ(c.chain_head_offset(), 42u);
    EXPECT_EQ(c.chain_length(), 2u);
}

// ─────────────────────────────────────────────────────────────────────────────
// ValueHandle (variant)
// ─────────────────────────────────────────────────────────────────────────────

TEST(ValueHandle, DefaultIsInline) {
    vh::ValueHandle<8> v;
    EXPECT_EQ(v.kind(), vh::HandleKind::Inline);
    EXPECT_TRUE(v.is_inline());
}

TEST(ValueHandle, ConstructFromExternal) {
    vh::ValueHandle<8> v{vh::ExternalHandle{4096, 128}};
    EXPECT_EQ(v.kind(), vh::HandleKind::External);
    EXPECT_TRUE(v.is_external());
    EXPECT_EQ(v.as_external().pool_offset(), 4096u);
}

TEST(ValueHandle, ConstructFromChainRef) {
    vh::ValueHandle<8> v{vh::ChainRefHandle{8192, 5}};
    EXPECT_EQ(v.kind(), vh::HandleKind::ChainRef);
    EXPECT_TRUE(v.is_chain_ref());
    EXPECT_EQ(v.as_chain_ref().chain_length(), 5u);
}

TEST(ValueHandle, VisitorPattern) {
    vh::ValueHandle<8> v{vh::ExternalHandle{100, 16}};
    int                hits[3] = {0, 0, 0};
    v.visit([&](auto const& h) {
        using T = std::decay_t<decltype(h)>;
        if constexpr (std::is_same_v<T, vh::InlineHandle<8>>)
            ++hits[0];
        else if constexpr (std::is_same_v<T, vh::ExternalHandle>)
            ++hits[1];
        else if constexpr (std::is_same_v<T, vh::ChainRefHandle>)
            ++hits[2];
    });
    EXPECT_EQ(hits[0], 0);
    EXPECT_EQ(hits[1], 1);
    EXPECT_EQ(hits[2], 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Cost-Model (H3-Hypothese)
// ─────────────────────────────────────────────────────────────────────────────

TEST(CostModel, InlineFitsCheck) {
    vh::WorkloadProfile w;
    EXPECT_TRUE(vh::inline_handle_fits(8, w));
    EXPECT_FALSE(vh::inline_handle_fits(9, w));
}

TEST(CostModel, RecommendChainRefForMultiValue) {
    vh::WorkloadProfile w;
    EXPECT_EQ(vh::recommend_handle(4, true, w), vh::HandleRecommendation::ChainRef);
}

TEST(CostModel, RecommendExternalForLargeValues) {
    vh::WorkloadProfile w;
    EXPECT_EQ(vh::recommend_handle(64, false, w), vh::HandleRecommendation::External);
}

TEST(CostModel, RecommendInlineForSmallSingleAccess) {
    vh::WorkloadProfile w;
    w.access_count_per_value = 1;
    auto r                   = vh::recommend_handle(4, false, w);
    EXPECT_EQ(r, vh::HandleRecommendation::Inline);
}

TEST(CostModel, InlineRemainsCheaperThanExternalForSmallValues) {
    // 8-Byte-Werte profitieren grundsaetzlich von Inline (vermeidet Pointer-Chase)
    // selbst bei hoher Access-Count. Das ist H3-Hypothese: kleine Werte → Inline.
    vh::WorkloadProfile w;
    w.access_count_per_value      = 100000;
    w.cache_line_pollution_factor = 5.0;
    w.pointer_chase_cycles        = 1.0;
    auto r                        = vh::recommend_handle(8, false, w);
    EXPECT_EQ(r, vh::HandleRecommendation::Inline);
}

TEST(CostModel, ExternalIsRecommendedOnceValueExceedsCapacity) {
    vh::WorkloadProfile w;
    w.inline_capacity_bytes = 8;
    EXPECT_EQ(vh::recommend_handle(9, false, w), vh::HandleRecommendation::External);
    EXPECT_EQ(vh::recommend_handle(64, false, w), vh::HandleRecommendation::External);
}

TEST(CostModel, EstimateCostsAreNonNegative) {
    vh::WorkloadProfile w;
    EXPECT_GT(vh::estimate_inline_cost(4, w), 0.0);
    EXPECT_GT(vh::estimate_external_cost(4, w), 0.0);
}
