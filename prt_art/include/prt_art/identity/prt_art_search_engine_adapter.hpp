#pragma once
// PrtArtSearchEngineAdapter — ABI-Inheritance Adapter (REV 7.6 V8.9)
//
// User-Direktive 2026-05-13/14 (STRUCTURAL_CORRECTION_diplomarbeit.md §4.2):
//   "Die CacheEngine sollte eigentlich die 8 Schichten an den PRT-ART
//    vererben, die es an Algorithmus-Bausteinen gibt. Ich sehe hier nicht
//    klar aufgefuehrt, dass der PRT-ART strikt die vorhandenen Strukturen
//    und source Interfaces der CacheEngine erweitert."
//
// Dieser Adapter ueberfuehrt die hybride PrtArtSearchEngine in das ABI-
// Interface comdare::search_engine (cache-engine ABI).
//
// Designentscheidung: KOMPOSITION statt direkter Vererbung im hybriden Template.
// Begruendung: Die hybride PrtArtSearchEngine hat 3 Spezialisierungen (Vector/Map/Tuple-API),
// die nicht alle die ABI-search_engine-Vertraege erfuellen koennen. Stattdessen
// haelt der Adapter eine PrtArtSearchEngine als Member und erbt von
// comdare::search_engine. Damit wird die ABI-Konformitaet gewahrt, ohne die
// hybride API zu brechen — die existierenden 51 Tests bleiben gruen.

#include <prt_art/identity/prt_art_search_engine.hpp>
#include <prt_art/identity/status.hpp>

#include <cache_engine/abi/configuration_permutation.hpp>
#include <cache_engine/abi/execution_engine.hpp>
#include <cache_engine/abi/processing_strategy.hpp>
#include <cache_engine/abi/search_algorithm_type_collection.hpp>
#include <cache_engine/abi/search_engine.hpp>

// REV 7.6 V8.11 — TestDataSetAccumulationEngine Constructor-Param
#if __has_include(<comdare/test_data_accumulation/test_data_set_accumulation_engine.hpp>)
  #include <comdare/test_data_accumulation/test_data_set_accumulation_engine.hpp>
  #define COMDARE_PRTART_HAS_TEST_DATA_SET 1
#endif

#include <memory>
#include <type_traits>

namespace comdare::prt_art::identity {

// PrtArt-spezifische ConfigurationPermutation (zur ABI-Konformitaet).
// Im V8 Skelett: leere Strategy. Folge-Phasen verdrahten hier die 8
// PRT-ART-Bausteine als compile-time std::variants.
struct PrtArtConfigurationPermutation {
    using strategy_t = ::comdare::processing_strategy<>;  // Default Template Args
    strategy_t strategy{};
};

// Adapter: PrtArtSearchEngine -> comdare::search_engine ABI
template <typename... Ts>
class PrtArtSearchEngineAdapter
    : public ::comdare::search_engine<
          ::comdare::search_algorithm_type_collection<Ts...>,
          PrtArtConfigurationPermutation>
{
public:
    using base_t = ::comdare::search_engine<
        ::comdare::search_algorithm_type_collection<Ts...>,
        PrtArtConfigurationPermutation>;
    using collection_t = typename base_t::collection_t;
    using config_t     = typename base_t::config_t;

    // Verdrahtet die hybride PrtArtSearchEngine im Konstruktor.
    explicit PrtArtSearchEngineAdapter(
        std::shared_ptr<::comdare::cache_engine::CacheEngine> ce,
        config_t cfg = {})
        : base_t{std::move(ce), std::move(cfg)},
          impl_{std::make_unique<PrtArtSearchEngine<Ts...>>()} {}

#ifdef COMDARE_PRTART_HAS_TEST_DATA_SET
    // REV 7.6 V8.11 — TestDataSet wird bei SearchEngine-Initialisierung
    // mit-konstruiert (User-Direktive REV 7 §10.11 / Master §10.11).
    template <typename SelfBindAlgo = PrtArtSearchEngineAdapter>
    PrtArtSearchEngineAdapter(
        std::shared_ptr<::comdare::cache_engine::CacheEngine> ce,
        ::comdare::test_data_accumulation::TestDataSetAccumulationEngine<SelfBindAlgo>& dataset_engine,
        config_t cfg = {})
        : base_t{std::move(ce), std::move(cfg)},
          impl_{std::make_unique<PrtArtSearchEngine<Ts...>>()},
          dataset_engine_ptr_{&dataset_engine} {}

    [[nodiscard]] bool has_dataset_engine() const noexcept {
        return dataset_engine_ptr_ != nullptr;
    }
#endif

    [[nodiscard]] PrtArtSearchEngine<Ts...>&       impl() noexcept       { return *impl_; }
    [[nodiscard]] PrtArtSearchEngine<Ts...> const& impl() const noexcept { return *impl_; }

private:
    std::unique_ptr<PrtArtSearchEngine<Ts...>> impl_;

#ifdef COMDARE_PRTART_HAS_TEST_DATA_SET
    void* dataset_engine_ptr_ = nullptr;  // type-erased, da template self-binding
#endif
};

// ═════════════════════════════════════════════════════════════════════════════
// REV 7.6 V9.1 — Konkrete Adapter-Subklassen
// Implementieren die abstract virtual lookup/insert/erase/size/empty per
// Delegation an impl_->find/insert/erase/size/empty.
// ═════════════════════════════════════════════════════════════════════════════

// Map-API Variante (2 Template-Parameter, K + V)
template <typename Key, typename Value>
class PrtArtSearchEngineAdapterMap final
    : public PrtArtSearchEngineAdapter<Key, Value>
{
public:
    using base_t  = PrtArtSearchEngineAdapter<Key, Value>;
    using key_t   = typename base_t::base_t::key_t;
    using value_t = typename base_t::base_t::value_t;
    using base_t::base_t;

    [[nodiscard]] std::optional<value_t> lookup(key_t const& k) override {
        return this->impl().find(k);
    }
    void insert(key_t const& k, value_t const& v) override {
        (void)this->impl().insert(k, v);  // status_t-Returncode ignoriert (search_engine API)
    }
    [[nodiscard]] bool erase(key_t const& k) override {
        return this->impl().erase(k) == status_ok;
    }
    [[nodiscard]] std::size_t size() const noexcept override {
        return this->impl().size();
    }
    [[nodiscard]] bool empty() const noexcept override {
        return this->impl().empty();
    }
};

// Vector-API Variante (1 Template-Parameter, nur V — Key implicit uint64 counter)
template <typename Value>
class PrtArtSearchEngineAdapterVector final
    : public PrtArtSearchEngineAdapter<Value>
{
public:
    using base_t  = PrtArtSearchEngineAdapter<Value>;
    using key_t   = typename base_t::base_t::key_t;     // std::uint64_t (auto-counter)
    using value_t = typename base_t::base_t::value_t;
    using base_t::base_t;

    [[nodiscard]] std::optional<value_t> lookup(key_t const& k) override {
        return this->impl().at(static_cast<std::size_t>(k));
    }
    void insert([[maybe_unused]] key_t const& k, value_t const& v) override {
        (void)this->impl().push_back(v);  // 1-Param-API: Key implicit
    }
    [[nodiscard]] bool erase([[maybe_unused]] key_t const& k) override {
        return this->impl().pop_back() == status_ok;  // simplified: 1-Param erase
    }
    [[nodiscard]] std::size_t size() const noexcept override {
        return this->impl().size();
    }
    [[nodiscard]] bool empty() const noexcept override {
        return this->impl().empty();
    }
};

// Tuple-API Variante (N>2 Template-Parameter, K + V1 + V2 + ...)
template <typename Key, typename V1, typename V2, typename... Rest>
class PrtArtSearchEngineAdapterTuple final
    : public PrtArtSearchEngineAdapter<Key, V1, V2, Rest...>
{
public:
    using base_t  = PrtArtSearchEngineAdapter<Key, V1, V2, Rest...>;
    using key_t   = typename base_t::base_t::key_t;
    using value_t = typename base_t::base_t::value_t;   // std::tuple<V1, V2, Rest...>
    using base_t::base_t;

    [[nodiscard]] std::optional<value_t> lookup(key_t const& k) override {
        return this->impl().find(k);
    }
    void insert(key_t const& k, value_t const& v) override {
        (void)this->impl().insert(k, v);
    }
    [[nodiscard]] bool erase(key_t const& k) override {
        return this->impl().erase(k) == status_ok;
    }
    [[nodiscard]] std::size_t size() const noexcept override {
        return this->impl().size();
    }
    [[nodiscard]] bool empty() const noexcept override {
        return this->impl().empty();
    }
};

}  // namespace comdare::prt_art::identity
