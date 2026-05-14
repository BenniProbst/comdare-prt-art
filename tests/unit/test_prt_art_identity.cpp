// Tests fuer hybride PrtArtSearchEngine (REV 7 Final, 2026-05-13)
// User-Direktive: Vector-API (1 Param), Map-API (2 Params), tuple-API (N>2 Params)
// Schreiboperatoren returnen status_t (int errno-style, 0=ok)

#include <prt_art/identity/prt_art_identity.hpp>
#include <prt_art/identity/prt_art_search_engine.hpp>
#include <prt_art/identity/status.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace id = comdare::prt_art::identity;
namespace ce = comdare::cache_engine;
using comdare::prt_art::status_ok;
using comdare::prt_art::status_key_already_exists;
using comdare::prt_art::status_key_not_found;
using comdare::prt_art::status_out_of_range;
using comdare::prt_art::status_empty_container;

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
// Spezialisierung B (Map-API, 2 Params): PrtArtSearchEngine<Key, Value>
// ─────────────────────────────────────────────────────────────────────────────

TEST(MapApi, EmptyAfterConstruction) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_TRUE(e.empty());
    EXPECT_EQ(e.size(), 0u);
    EXPECT_EQ(e.total_inserts(), 0u);
}

TEST(MapApi, IdentityFlagsMatchFreeFunction) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.identity_flags(), id::prt_art_permutation_flags());
}

TEST(MapApi, IdentifierMatchesFreeFunction) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.identifier(), id::prt_art_identifier());
}

TEST(MapApi, PoolsConfiguredFor7Slots) {
    id::PrtArtSearchEngine<int, int> e;
    using ce_pool = comdare::prt_art::allocator::PoolKind;
    EXPECT_EQ(e.pools().get(ce_pool::A_TrieHuelle).slot_size_bytes,    64u);
    EXPECT_EQ(e.pools().get(ce_pool::B_DensePages).slot_size_bytes,   256u);
    EXPECT_EQ(e.pools().get(ce_pool::C_MultiLevel).slot_size_bytes,  1024u);
    EXPECT_EQ(e.pools().get(ce_pool::D_DecisionSpan).slot_size_bytes, 4096u);
    EXPECT_EQ(e.pools().get(ce_pool::V_StaticValue).slot_size_bytes,   16u);
}

TEST(MapApi, InsertReturnsStatusOk) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.insert(1, 100), status_ok);
    EXPECT_EQ(e.insert(2, 200), status_ok);
    EXPECT_EQ(e.total_inserts(), 2u);
}

TEST(MapApi, InsertExistingReturnsKeyAlreadyExists) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.insert(5, 50),  status_ok);
    EXPECT_EQ(e.insert(5, 500), status_key_already_exists);
    EXPECT_EQ(e.size(), 1u);
    auto v = e.lookup(5);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 50);
}

TEST(MapApi, SetUpdatesExistingKey) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.set(5, 50),  status_ok);
    EXPECT_EQ(e.set(5, 500), status_ok);
    EXPECT_EQ(e.size(), 1u);
    auto v = e.lookup(5);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 500);
}

TEST(MapApi, LookupOnEmptyReturnsNullopt) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_FALSE(e.lookup(42).has_value());
}

TEST(MapApi, LookupAfterInsertRoundtrip) {
    id::PrtArtSearchEngine<int, int> e;
    e.insert(42, 4242);
    e.insert(7,  77);
    auto v1 = e.lookup(42);
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(*v1, 4242);
    auto v2 = e.lookup(7);
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(*v2, 77);
}

TEST(MapApi, EraseRemovesEntryAndReturnsOk) {
    id::PrtArtSearchEngine<int, int> e;
    e.insert(11, 1100);
    e.insert(22, 2200);
    EXPECT_EQ(e.erase(11), status_ok);
    EXPECT_EQ(e.size(), 1u);
    EXPECT_FALSE(e.lookup(11).has_value());
    EXPECT_EQ(e.total_erases(), 1u);
}

TEST(MapApi, EraseNonExistentReturnsKeyNotFound) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.erase(0),   status_key_not_found);
    EXPECT_EQ(e.erase(123), status_key_not_found);
    EXPECT_EQ(e.total_erases(), 0u);
}

TEST(MapApi, ClearRemovesAll) {
    id::PrtArtSearchEngine<int, int> e;
    for (int i = 0; i < 50; ++i) e.insert(i, i);
    EXPECT_EQ(e.size(), 50u);
    EXPECT_EQ(e.clear(), status_ok);
    EXPECT_EQ(e.size(), 0u);
}

TEST(MapApi, ContainsAndCount) {
    id::PrtArtSearchEngine<int, int> e;
    e.insert(7, 77);
    EXPECT_TRUE(e.contains(7));
    EXPECT_FALSE(e.contains(8));
    EXPECT_EQ(e.count(7), 1u);
    EXPECT_EQ(e.count(8), 0u);
}

TEST(MapApi, RangeScanReturnsNonEmpty) {
    id::PrtArtSearchEngine<int, int> e;
    for (int i = 0; i < 20; ++i) e.insert(i, i * 100);
    auto result = e.range_scan(5, 10);
    EXPECT_LE(result.size(), 10u);
    EXPECT_FALSE(result.empty());
}

TEST(MapApi, StringKeyAndStringValueWorks) {
    id::PrtArtSearchEngine<std::string, std::string> e;
    EXPECT_EQ(e.insert("alpha", "first"),  status_ok);
    EXPECT_EQ(e.insert("beta",  "second"), status_ok);
    auto v = e.lookup(std::string{"alpha"});
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "first");
    EXPECT_EQ(e.erase(std::string{"alpha"}), status_ok);
    EXPECT_FALSE(e.lookup(std::string{"alpha"}).has_value());
}

TEST(MapApi, DensityTrackerReceivesUpdatesOnInserts) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_EQ(e.density_tracker().total_observations(), 0u);
    for (int i = 0; i < 50; ++i) e.insert(i, i);
    EXPECT_GT(e.density_tracker().total_observations(), 0u);
}

TEST(MapApi, OlcReservedBlocksGrowWithWrites) {
    id::PrtArtSearchEngine<int, int> e;
    auto const before = e.concurrency().total_blocks_reserved();
    for (int i = 0; i < 16; ++i) e.insert(i, i * 7);
    auto const after = e.concurrency().total_blocks_reserved();
    EXPECT_GE(after - before, 16u);
}

TEST(MapApi, MultiThreadedInsertsAreConsistent) {
    id::PrtArtSearchEngine<int, int> e;
    constexpr int kThreads          = 4;
    constexpr int kPerThreadInserts = 250;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([t, &e]() {
            for (int i = 0; i < kPerThreadInserts; ++i) {
                int const key = t * kPerThreadInserts + i;
                (void)e.insert(key, key + 1000);
            }
        });
    }
    for (auto& th : threads) th.join();

    EXPECT_EQ(e.size(), static_cast<std::size_t>(kThreads * kPerThreadInserts));
    for (int i = 0; i < kPerThreadInserts; ++i) {
        auto v = e.lookup(i);
        ASSERT_TRUE(v.has_value()) << "Missing key " << i;
        EXPECT_EQ(*v, i + 1000);
    }
}

TEST(MapApi, HitMissCountersTracked) {
    id::PrtArtSearchEngine<int, int> e;
    e.insert(1, 11);
    e.insert(2, 22);
    (void)e.lookup(1);
    (void)e.lookup(2);
    (void)e.lookup(3);
    (void)e.lookup(4);
    (void)e.lookup(5);
    EXPECT_EQ(e.total_hits(),   2u);
    EXPECT_EQ(e.total_misses(), 3u);
}

TEST(MapApi, ComponentAccessorsAreReachable) {
    id::PrtArtSearchEngine<int, int> e;
    EXPECT_NO_THROW((void)e.pools());
    EXPECT_NO_THROW((void)e.concurrency());
    EXPECT_NO_THROW((void)e.memory_layout());
    EXPECT_NO_THROW((void)e.path_prefetch());
    EXPECT_NO_THROW((void)e.density_tracker());
    EXPECT_NO_THROW((void)e.hypothesis_metrics());
}

// ─────────────────────────────────────────────────────────────────────────────
// Spezialisierung B (Tuple-API, N>2 Params): PrtArtSearchEngine<Key, V1, V2, ...>
// ─────────────────────────────────────────────────────────────────────────────

TEST(TupleApi, MappedTypeIsTuple) {
    using Engine = id::PrtArtSearchEngine<int, int, double, std::string>;
    static_assert(std::is_same_v<Engine::mapped_type,
                                  std::tuple<int, double, std::string>>);
    EXPECT_EQ(Engine::kValueArity, 3u);
}

TEST(TupleApi, InsertWithMultipleValuesPacksIntoTuple) {
    id::PrtArtSearchEngine<int, int, double, std::string> e;
    EXPECT_EQ(e.insert(1, 10, 1.5, std::string{"one"}), status_ok);
    EXPECT_EQ(e.insert(2, 20, 2.5, std::string{"two"}), status_ok);
    EXPECT_EQ(e.size(), 2u);

    auto v = e.lookup(1);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(std::get<0>(*v), 10);
    EXPECT_DOUBLE_EQ(std::get<1>(*v), 1.5);
    EXPECT_EQ(std::get<2>(*v), "one");
}

TEST(TupleApi, DuplicateInsertReturnsKeyAlreadyExists) {
    id::PrtArtSearchEngine<int, int, double> e;
    EXPECT_EQ(e.insert(1, 10, 1.5),  status_ok);
    EXPECT_EQ(e.insert(1, 20, 2.5),  status_key_already_exists);
    auto v = e.lookup(1);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(std::get<0>(*v), 10);
}

TEST(TupleApi, EraseInTupleMode) {
    id::PrtArtSearchEngine<int, int, double> e;
    e.insert(1, 10, 1.5);
    e.insert(2, 20, 2.5);
    EXPECT_EQ(e.erase(1), status_ok);
    EXPECT_FALSE(e.lookup(1).has_value());
    EXPECT_EQ(e.size(), 1u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Spezialisierung A (Vector-API, 1 Param): PrtArtSearchEngine<Value>
// ─────────────────────────────────────────────────────────────────────────────

TEST(VectorApi, EmptyAfterConstruction) {
    id::PrtArtSearchEngine<int> v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}

TEST(VectorApi, PushBackReturnsStatusOk) {
    id::PrtArtSearchEngine<int> v;
    EXPECT_EQ(v.push_back(10), status_ok);
    EXPECT_EQ(v.push_back(20), status_ok);
    EXPECT_EQ(v.push_back(30), status_ok);
    EXPECT_EQ(v.size(), 3u);
}

TEST(VectorApi, AtAndFrontBackReadAccess) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    auto a0 = v.at(0);
    ASSERT_TRUE(a0.has_value());
    EXPECT_EQ(*a0, 10);

    auto a2 = v.at(2);
    ASSERT_TRUE(a2.has_value());
    EXPECT_EQ(*a2, 30);

    auto f = v.front();
    ASSERT_TRUE(f.has_value());
    EXPECT_EQ(*f, 10);

    auto b = v.back();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(*b, 30);
}

TEST(VectorApi, AtOutOfRangeReturnsNullopt) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(10);
    EXPECT_FALSE(v.at(5).has_value());
}

TEST(VectorApi, FrontBackOnEmptyReturnNullopt) {
    id::PrtArtSearchEngine<int> v;
    EXPECT_FALSE(v.front().has_value());
    EXPECT_FALSE(v.back().has_value());
}

TEST(VectorApi, PopBackRemovesLast) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(10);
    v.push_back(20);
    EXPECT_EQ(v.pop_back(), status_ok);
    EXPECT_EQ(v.size(), 1u);
    auto b = v.back();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(*b, 10);
}

TEST(VectorApi, PopBackOnEmptyReturnsEmptyContainer) {
    id::PrtArtSearchEngine<int> v;
    EXPECT_EQ(v.pop_back(), status_empty_container);
}

TEST(VectorApi, SetAtUpdatesElement) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(10);
    v.push_back(20);
    EXPECT_EQ(v.set_at(1, 99), status_ok);
    auto a1 = v.at(1);
    ASSERT_TRUE(a1.has_value());
    EXPECT_EQ(*a1, 99);
}

TEST(VectorApi, SetAtOutOfRangeReturnsError) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(10);
    EXPECT_EQ(v.set_at(5, 99), status_out_of_range);
}

TEST(VectorApi, InsertAtInTheMiddle) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(10);
    v.push_back(30);
    EXPECT_EQ(v.insert_at(1, 20), status_ok);
    auto a0 = v.at(0);  ASSERT_TRUE(a0.has_value()); EXPECT_EQ(*a0, 10);
    auto a1 = v.at(1);  ASSERT_TRUE(a1.has_value()); EXPECT_EQ(*a1, 20);
    auto a2 = v.at(2);  ASSERT_TRUE(a2.has_value()); EXPECT_EQ(*a2, 30);
}

TEST(VectorApi, EraseAtRemovesElement) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    EXPECT_EQ(v.erase_at(1), status_ok);
    EXPECT_EQ(v.size(), 2u);
    auto a0 = v.at(0);  ASSERT_TRUE(a0.has_value()); EXPECT_EQ(*a0, 10);
    auto a1 = v.at(1);  ASSERT_TRUE(a1.has_value()); EXPECT_EQ(*a1, 30);
}

TEST(VectorApi, ClearAndReserveAndResize) {
    id::PrtArtSearchEngine<int> v;
    for (int i = 0; i < 10; ++i) v.push_back(i);
    EXPECT_EQ(v.size(), 10u);

    EXPECT_EQ(v.reserve(100), status_ok);
    EXPECT_GE(v.capacity(), 100u);

    EXPECT_EQ(v.clear(), status_ok);
    EXPECT_TRUE(v.empty());

    EXPECT_EQ(v.resize(5, 42), status_ok);
    EXPECT_EQ(v.size(), 5u);
    auto a0 = v.at(0);  ASSERT_TRUE(a0.has_value()); EXPECT_EQ(*a0, 42);
    auto a4 = v.at(4);  ASSERT_TRUE(a4.has_value()); EXPECT_EQ(*a4, 42);
}

TEST(VectorApi, AssignReplacesAll) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    EXPECT_EQ(v.assign(5, 99), status_ok);
    EXPECT_EQ(v.size(), 5u);
    auto a0 = v.at(0);  ASSERT_TRUE(a0.has_value()); EXPECT_EQ(*a0, 99);
}

TEST(VectorApi, ShrinkToFitOk) {
    id::PrtArtSearchEngine<int> v;
    v.reserve(1000);
    v.push_back(1);
    EXPECT_EQ(v.shrink_to_fit(), status_ok);
}

TEST(VectorApi, DataPointerAccess) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(11);
    v.push_back(22);
    auto* p = v.data();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p[0], 11);
    EXPECT_EQ(p[1], 22);
}

TEST(VectorApi, IteratorsTraverse) {
    id::PrtArtSearchEngine<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    int sum = 0;
    for (auto it = v.begin(); it != v.end(); ++it) sum += *it;
    EXPECT_EQ(sum, 6);
}

TEST(VectorApi, IdentityFlagsMatchEvenInVectorMode) {
    id::PrtArtSearchEngine<int> v;
    EXPECT_EQ(v.identity_flags(), id::prt_art_permutation_flags());
}

TEST(VectorApi, ComponentAccessorsAreReachable) {
    id::PrtArtSearchEngine<int> v;
    EXPECT_NO_THROW((void)v.pools());
    EXPECT_NO_THROW((void)v.concurrency());
    EXPECT_NO_THROW((void)v.memory_layout());
    EXPECT_NO_THROW((void)v.path_prefetch());
    EXPECT_NO_THROW((void)v.density_tracker());
    EXPECT_NO_THROW((void)v.hypothesis_metrics());
}

TEST(VectorApi, MultiThreadedPushBackIsConsistent) {
    id::PrtArtSearchEngine<int> v;
    constexpr int kThreads          = 4;
    constexpr int kPerThreadInserts = 250;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([t, &v]() {
            for (int i = 0; i < kPerThreadInserts; ++i) {
                (void)v.push_back(t * kPerThreadInserts + i);
            }
        });
    }
    for (auto& th : threads) th.join();

    EXPECT_EQ(v.size(),
              static_cast<std::size_t>(kThreads * kPerThreadInserts));
}

// ─────────────────────────────────────────────────────────────────────────────
// Status-Codes Sanity
// ─────────────────────────────────────────────────────────────────────────────

TEST(Status, ConstantsAreDistinct) {
    using namespace comdare::prt_art;
    EXPECT_EQ(status_ok, 0);
    EXPECT_NE(status_key_already_exists, status_ok);
    EXPECT_NE(status_key_not_found,      status_ok);
    EXPECT_NE(status_out_of_range,       status_ok);
    EXPECT_NE(status_empty_container,    status_ok);
    EXPECT_TRUE(status_is_ok(status_ok));
    EXPECT_TRUE(status_is_error(status_key_not_found));
}

// ═════════════════════════════════════════════════════════════════════════════
// REV 7.6 V14.1 — Tests fuer V12.1 Vector-API Erweiterungen
// ═════════════════════════════════════════════════════════════════════════════

TEST(VectorApi_V12, OperatorBracketReturnsValueAtIndex) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int> v;
    EXPECT_EQ(v.push_back(10), status_ok);
    EXPECT_EQ(v.push_back(20), status_ok);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
}

TEST(VectorApi_V12, EmplaceBackWithArgs) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<std::string> v;
    EXPECT_EQ(v.emplace_back(5, 'a'), status_ok);
    auto front = v.front();
    ASSERT_TRUE(front.has_value());
    EXPECT_EQ(*front, "aaaaa");
}

TEST(VectorApi_V12, SwapTwoEngines) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int> a;
    PrtArtSearchEngine<int> b;
    EXPECT_EQ(a.push_back(1), status_ok);
    EXPECT_EQ(a.push_back(2), status_ok);
    EXPECT_EQ(b.push_back(99), status_ok);
    a.swap(b);
    EXPECT_EQ(a.size(), 1u);
    EXPECT_EQ(b.size(), 2u);
    EXPECT_EQ(a[0], 99);
    EXPECT_EQ(b[0], 1);
}

TEST(VectorApi_V12, ReverseIteratorsTraverseInReverseOrder) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int> v;
    EXPECT_EQ(v.push_back(1), status_ok);
    EXPECT_EQ(v.push_back(2), status_ok);
    EXPECT_EQ(v.push_back(3), status_ok);
    std::vector<int> reversed;
    for (auto it = v.rbegin(); it != v.rend(); ++it) reversed.push_back(*it);
    EXPECT_EQ(reversed.size(), 3u);
    EXPECT_EQ(reversed[0], 3);
    EXPECT_EQ(reversed[2], 1);
}

TEST(VectorApi_V12, AssignFromRange) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int> v;
    std::vector<int> source{10, 20, 30, 40};
    EXPECT_EQ(v.assign(source.begin(), source.end()), status_ok);
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[3], 40);
}

// ═════════════════════════════════════════════════════════════════════════════
// REV 7.6 V14.1 — Tests fuer V12.2 Map-API Erweiterungen
// ═════════════════════════════════════════════════════════════════════════════

TEST(MapApi_V12, OperatorBracketLookup) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> m;
    EXPECT_EQ(m.insert(1, "one"), status_ok);
    auto v = m[1];
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "one");
    auto missing = m[42];
    EXPECT_FALSE(missing.has_value());
}

TEST(MapApi_V12, EmplaceWithArgs) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> m;
    EXPECT_EQ(m.emplace(7, 3, 'x'), status_ok);  // string(3, 'x') = "xxx"
    auto v = m.find(7);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "xxx");
}

TEST(MapApi_V12, TryEmplaceFailsOnExistingKey) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> m;
    EXPECT_EQ(m.try_emplace(1, std::string("first")),  status_ok);
    EXPECT_EQ(m.try_emplace(1, std::string("second")), status_key_already_exists);
}

TEST(MapApi_V12, InsertOrAssignOverwrites) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> m;
    EXPECT_EQ(m.insert(1, "first"), status_ok);
    EXPECT_EQ(m.insert_or_assign(1, "second"), status_ok);
    auto v = m.find(1);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "second");
}

TEST(MapApi_V12, SwapTwoMaps) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> a;
    PrtArtSearchEngine<int, std::string> b;
    EXPECT_EQ(a.insert(1, "a1"), status_ok);
    EXPECT_EQ(b.insert(2, "b2"), status_ok);
    a.swap(b);
    EXPECT_EQ(a.size(), 1u);
    EXPECT_EQ(b.size(), 1u);
    EXPECT_TRUE(a.contains(2));
    EXPECT_TRUE(b.contains(1));
}

TEST(MapApi_V12, MaxSizeIsPositive) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, int> m;
    EXPECT_GT(m.max_size(), 0u);
}

TEST(MapApi_V12, KeyCompAndValueCompCallable) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, int> m;
    auto kc = m.key_comp();
    auto vc = m.value_comp();
    (void)kc; (void)vc;  // verifiziert nur dass sie compile-bar sind
    SUCCEED();
}

// ═════════════════════════════════════════════════════════════════════════════
// REV 7.6 V14.1 — Tests fuer V13.5 merge + extract
// ═════════════════════════════════════════════════════════════════════════════

TEST(MapApi_V13, MergeTakesOnlyUniqueKeys) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> a;
    PrtArtSearchEngine<int, std::string> b;
    EXPECT_EQ(a.insert(1, "a1"), status_ok);
    EXPECT_EQ(a.insert(2, "a2"), status_ok);
    EXPECT_EQ(b.insert(2, "b2"), status_ok);  // Duplikat -> wird NICHT gemerged
    EXPECT_EQ(b.insert(3, "b3"), status_ok);  // unique -> wird gemerged
    EXPECT_EQ(b.insert(4, "b4"), status_ok);  // unique -> wird gemerged
    auto merged = a.merge(b);
    EXPECT_EQ(merged, 2u);  // 3+4 gemerged, 2 nicht
    EXPECT_EQ(a.size(), 4u);
    EXPECT_EQ(b.size(), 1u);  // nur key=2 bleibt
    EXPECT_TRUE(b.contains(2));
}

TEST(MapApi_V13, ExtractReturnsValueAndRemoves) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> m;
    EXPECT_EQ(m.insert(42, "hello"), status_ok);
    EXPECT_EQ(m.size(), 1u);
    auto extracted = m.extract(42);
    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(*extracted, "hello");
    EXPECT_EQ(m.size(), 0u);
    EXPECT_FALSE(m.contains(42));
}

TEST(MapApi_V13, ExtractMissingKeyReturnsNullopt) {
    using namespace comdare::prt_art::identity;
    PrtArtSearchEngine<int, std::string> m;
    auto extracted = m.extract(999);
    EXPECT_FALSE(extracted.has_value());
}
