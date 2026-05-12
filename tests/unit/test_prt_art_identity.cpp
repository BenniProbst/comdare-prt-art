// Tests fuer PRT-ART Identitaet (REV 6 Final)
// PrtArtSearchEngine komponiert alle Bausteine + liefert PermutationFlags-Identifier

#include <prt_art/identity/prt_art_identity.hpp>
#include <prt_art/identity/prt_art_search_engine.hpp>

#include <gtest/gtest.h>

namespace id = comdare::prt_art::identity;
namespace ce = comdare::cache_engine;

// ─────────────────────────────────────────────────────────────────────────────
// PrtArtIdentity (PermutationFlags-Tag)
// ─────────────────────────────────────────────────────────────────────────────

TEST(PrtArtIdentity, FlagsAreNonEmpty) {
    auto f = id::prt_art_permutation_flags();
    EXPECT_TRUE(f.any_set());
}

TEST(PrtArtIdentity, FlagsAreValidCombination) {
    auto f = id::prt_art_permutation_flags();
    EXPECT_TRUE(f.is_valid_combination());
}

TEST(PrtArtIdentity, FlagsHaveAllBanksFilled) {
    auto f = id::prt_art_permutation_flags();
    EXPECT_NE(f.page_bank,         0u);
    EXPECT_NE(f.node_bank,         0u);
    EXPECT_NE(f.traversal_bank,    0u);
    EXPECT_NE(f.value_handle_bank, 0u);
    EXPECT_NE(f.memory_layout_bank,0u);
    EXPECT_NE(f.allocator_bank,    0u);
    EXPECT_NE(f.prefetch_bank,     0u);
    EXPECT_NE(f.concurrency_bank,  0u);
    EXPECT_NE(f.isa_bank,          0u);
    EXPECT_NE(f.telemetry_bank,    0u);
}

TEST(PrtArtIdentity, IdentifierIsHexString) {
    auto s = id::prt_art_identifier();
    // 10 Banks * 16 Hex-Stellen + 9 Trenner = 169
    EXPECT_EQ(s.size(), 169u);
}

TEST(PrtArtIdentity, FlagsContainPrtartSpecificPageBits) {
    auto f = id::prt_art_permutation_flags();
    using namespace ce::flags;
    EXPECT_NE(f.page_bank & page_bank::PRTART_REDIRECT,        0u);
    EXPECT_NE(f.page_bank & page_bank::PRTART_DENSEBYTE,       0u);
    EXPECT_NE(f.page_bank & page_bank::PRTART_SPARSEPATRICIA,  0u);
}

TEST(PrtArtIdentity, ContainsPrtArtBPlusAndRedirectNodes) {
    auto f = id::prt_art_permutation_flags();
    using namespace ce::flags;
    EXPECT_NE(f.node_bank & node_bank::PRTART_REDIRECT, 0u);
    EXPECT_NE(f.node_bank & node_bank::PRTART_BPLUS,    0u);
}

TEST(PrtArtIdentity, OlcConcurrencyIsSet) {
    auto f = id::prt_art_permutation_flags();
    using namespace ce::flags;
    EXPECT_NE(f.concurrency_bank & concurrency_bank::OLC, 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// PrtArtSearchEngine (Komposition)
// ─────────────────────────────────────────────────────────────────────────────

TEST(PrtArtSearchEngine, EmptyAfterConstruction) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_TRUE(e.empty());
    EXPECT_EQ(e.size(), 0u);
    EXPECT_EQ(e.total_inserts(), 0u);
}

TEST(PrtArtSearchEngine, IdentityFlagsMatchFreeFunction) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.identity_flags(), id::prt_art_permutation_flags());
}

TEST(PrtArtSearchEngine, IdentifierMatchesFreeFunction) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.identifier(), id::prt_art_identifier());
}

TEST(PrtArtSearchEngine, PoolsConfiguredFor7Slots) {
    id::PrtArtSearchEngine<int, int> e;
    using ce_pool = comdare::prt_art::allocator::PoolKind;
    EXPECT_EQ(e.pools().get(ce_pool::A_TrieHuelle).slot_size_bytes,    64u);
    EXPECT_EQ(e.pools().get(ce_pool::B_DensePages).slot_size_bytes,   256u);
    EXPECT_EQ(e.pools().get(ce_pool::C_MultiLevel).slot_size_bytes,  1024u);
    EXPECT_EQ(e.pools().get(ce_pool::D_DecisionSpan).slot_size_bytes, 4096u);
    EXPECT_EQ(e.pools().get(ce_pool::V_StaticValue).slot_size_bytes,   16u);
}

TEST(PrtArtSearchEngine, InsertIncrementsCounter) {
    id::PrtArtSearchEngine<int, int> e;
    e.insert(1, 100);
    e.insert(2, 200);
    EXPECT_EQ(e.total_inserts(), 2u);
}

TEST(PrtArtSearchEngine, LookupOnEmptyReturnsNullopt) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_FALSE(e.lookup(42).has_value());
}

TEST(PrtArtSearchEngine, ComponentAccessorsAreReachable) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_NO_THROW((void)e.pools());
    EXPECT_NO_THROW((void)e.concurrency());
    EXPECT_NO_THROW((void)e.memory_layout());
    EXPECT_NO_THROW((void)e.path_prefetch());
    EXPECT_NO_THROW((void)e.density_tracker());
    EXPECT_NO_THROW((void)e.hypothesis_metrics());
}
