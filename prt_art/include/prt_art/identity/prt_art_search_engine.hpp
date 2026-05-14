#pragma once
// PrtArtSearchEngine - HYBRIDE abstrakte Klasse (REV 7 Final, 2026-05-13)
//
// User-Direktive 2026-05-13: die SearchEngine ist eine hybride abstrakte
// Klasse, welche
//   - bei 1 Parameter  ALLE Funktionsinterfaces einer std::vector,
//   - bei 2 Parametern ALLE Funktionsinterfaces einer std::map,
//   - bei N>2 Parametern map<K, tuple<V1..VN>>
// fordert. Schreiboperatoren returnen IMMER error number (errno-style int,
// 0=ok, >0=Fehlercode; siehe status.hpp).
//
// Komponiert alle PRT-ART-Bausteine (4+2 Pools, OLC, MultiLevelLayout,
// PathOrientedPrefetch, DensityTracker, HypothesisMetrics).

#include <prt_art/allocator/pool_set.hpp>
#include <prt_art/concurrency/olc_with_reserved_blocks.hpp>
#include <prt_art/identity/prt_art_identity.hpp>
#include <prt_art/identity/status.hpp>
#include <prt_art/measurement/density_tracker.hpp>
#include <prt_art/measurement/hypothesis_metrics.hpp>
#include <prt_art/memory_layout/multi_level_layout.hpp>
#include <prt_art/prefetch/path_oriented_prefetch.hpp>
#include <prt_art/value_handle/value_handle.hpp>

#include <cache_engine/fingerprint/fixed_length_fingerprint.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace comdare::prt_art::identity {

// ─────────────────────────────────────────────────────────────────────────────
// Komponenten-Bundle: alle PRT-ART-Bausteine, gemeinsam von beiden
// Spezialisierungen verwendet (DRY)
// REV 7.6 V11.1 — alle Member sind public exposed fuer Adapter-Verdrahtung
// ─────────────────────────────────────────────────────────────────────────────
struct PrtArtComponents {
    ::comdare::prt_art::allocator::PoolSet                       pools{};
    ::comdare::prt_art::concurrency::OlcWithReservedValueBlocks  concurrency{};
    ::comdare::prt_art::memory_layout::MultiLevelLayout          memory_layout{};
    ::comdare::prt_art::prefetch::PathOrientedPrefetch           path_prefetch{};
    ::comdare::prt_art::measurement::DensityTracker              density_tracker{};
    ::comdare::prt_art::measurement::PrtArtHypothesisMetrics     hypothesis_metrics{};

    PrtArtComponents() {
        using PK = ::comdare::prt_art::allocator::PoolKind;
        pools.configure(PK::A_TrieHuelle,    64,   1024);
        pools.configure(PK::B_DensePages,   256,   1024);
        pools.configure(PK::C_MultiLevel,  1024,    512);
        pools.configure(PK::D_DecisionSpan, 4096,   256);
        pools.configure(PK::R_Rest,           0,      0);
        pools.configure(PK::V_StaticValue,   16,   8192);
        pools.configure(PK::V_DynamicValue,   0,   2048);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Primary Template: undefiniert, zwingt zur Spezialisierung
// ─────────────────────────────────────────────────────────────────────────────
template <typename... Ts>
class PrtArtSearchEngine;

// ═════════════════════════════════════════════════════════════════════════════
// SPEZIALISIERUNG A: 1 Template-Parameter  ─  std::vector-API
// ═════════════════════════════════════════════════════════════════════════════
template <typename Value>
class PrtArtSearchEngine<Value> {
public:
    using value_type      = Value;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = Value&;
    using const_reference = Value const&;
    using pointer         = Value*;
    using const_pointer   = Value const*;
    using iterator        = typename std::vector<Value>::iterator;
    using const_iterator  = typename std::vector<Value>::const_iterator;

    PrtArtSearchEngine() : components_{} {}

    // ─── Capacity (Lese-Ops) ─────────────────────────────────────────────────
    [[nodiscard]] size_type size()     const noexcept { return data_.size(); }
    [[nodiscard]] bool      empty()    const noexcept { return data_.empty(); }
    [[nodiscard]] size_type capacity() const noexcept { return data_.capacity(); }
    [[nodiscard]] size_type max_size() const noexcept { return data_.max_size(); }

    // ─── Element-Zugriff (Read = T& bzw. optional) ─────────────────────────
    [[nodiscard]] std::optional<Value> at(size_type index) const {
        std::shared_lock guard{rw_lock_};
        if (index >= data_.size()) return std::nullopt;
        return data_[index];
    }
    [[nodiscard]] std::optional<Value> front() const {
        std::shared_lock guard{rw_lock_};
        if (data_.empty()) return std::nullopt;
        return data_.front();
    }
    [[nodiscard]] std::optional<Value> back() const {
        std::shared_lock guard{rw_lock_};
        if (data_.empty()) return std::nullopt;
        return data_.back();
    }
    [[nodiscard]] const_pointer data() const noexcept { return data_.data(); }

    // Iterator-Zugriff (read-only)
    [[nodiscard]] const_iterator begin() const noexcept { return data_.cbegin(); }
    [[nodiscard]] const_iterator end()   const noexcept { return data_.cend(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return data_.cbegin(); }
    [[nodiscard]] const_iterator cend()   const noexcept { return data_.cend(); }

    // ─── Schreib-Ops (returnen int errno-style) ─────────────────────────────
    status_t push_back(Value const& v) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        try {
            data_.push_back(v);
            update_density();
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    status_t push_back(Value&& v) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        try {
            data_.push_back(std::move(v));
            update_density();
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    status_t pop_back() {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        if (data_.empty()) return status_empty_container;
        data_.pop_back();
        return status_ok;
    }

    status_t set_at(size_type index, Value const& v) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        if (index >= data_.size()) return status_out_of_range;
        data_[index] = v;
        return status_ok;
    }

    status_t insert_at(size_type index, Value const& v) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        if (index > data_.size()) return status_out_of_range;
        try {
            data_.insert(data_.begin() + index, v);
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    status_t erase_at(size_type index) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        if (index >= data_.size()) return status_out_of_range;
        data_.erase(data_.begin() + index);
        return status_ok;
    }

    status_t clear() {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        data_.clear();
        return status_ok;
    }

    status_t resize(size_type n) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        try {
            data_.resize(n);
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    status_t resize(size_type n, Value const& v) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        try {
            data_.resize(n, v);
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    status_t reserve(size_type n) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        try {
            data_.reserve(n);
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    status_t shrink_to_fit() {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        data_.shrink_to_fit();
        return status_ok;
    }

    status_t assign(size_type n, Value const& v) {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        try {
            data_.assign(n, v);
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    // ─── Identity API ─────────────────────────────────────────────────────────
    [[nodiscard]] static ::comdare::cache_engine::PermutationFlags identity_flags() noexcept {
        return prt_art_permutation_flags();
    }
    [[nodiscard]] static std::string identifier() {
        return prt_art_identifier();
    }

    // ─── Komponenten-Accessors ───────────────────────────────────────────────
    [[nodiscard]] auto& pools()              noexcept { return components_.pools; }
    [[nodiscard]] auto& concurrency()        noexcept { return components_.concurrency; }
    [[nodiscard]] auto& memory_layout()      noexcept { return components_.memory_layout; }
    [[nodiscard]] auto& path_prefetch()      noexcept { return components_.path_prefetch; }
    [[nodiscard]] auto& density_tracker()    noexcept { return components_.density_tracker; }
    [[nodiscard]] auto& hypothesis_metrics() noexcept { return components_.hypothesis_metrics; }

private:
    void update_density() {
        std::uint64_t const synthetic_node_id =
            static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(data_.data()) & 0xFFFFu);
        double const density_pct = (data_.size() * 100.0)
            / static_cast<double>((std::max<std::size_t>)(1, data_.capacity()));
        components_.density_tracker.record(synthetic_node_id, density_pct);
    }

    mutable std::shared_mutex rw_lock_;
    std::vector<Value>        data_;
    PrtArtComponents          components_;
};

// ═════════════════════════════════════════════════════════════════════════════
// SPEZIALISIERUNG B: 2+ Template-Parameter  ─  std::map-API
//   N==2 :  PrtArtSearchEngine<Key, Value>            -> value_t = Value
//   N >2 :  PrtArtSearchEngine<Key, V1, V2, ...>      -> value_t = tuple<V1..VN>
// ═════════════════════════════════════════════════════════════════════════════
template <typename Key, typename First, typename... Rest>
class PrtArtSearchEngine<Key, First, Rest...> {
public:
    static constexpr std::size_t kValueArity = 1 + sizeof...(Rest);

    using key_type        = Key;
    using mapped_type     = std::conditional_t<
                                kValueArity == 1,
                                First,
                                std::tuple<First, Rest...>>;
    using value_type      = std::pair<Key const, mapped_type>;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using binary_key_t    = ::comdare::fingerprint::BinaryKey;

private:
    // Speichermodell: map<binary_key, pair<original_key, mapped_value>>
    // binary_key sortiert; original_key fuer Iterator-Returns; mapped_value ist Wert (V oder tuple)
    using storage_value_t = std::pair<Key, mapped_type>;
    using storage_map_t   = std::map<binary_key_t, storage_value_t>;

public:
    using iterator       = typename storage_map_t::iterator;
    using const_iterator = typename storage_map_t::const_iterator;

    PrtArtSearchEngine() : components_{} {}

    // ─── Capacity (Lese-Ops) ─────────────────────────────────────────────────
    [[nodiscard]] size_type size()  const noexcept { return storage_.size(); }
    [[nodiscard]] bool      empty() const noexcept { return storage_.empty(); }
    [[nodiscard]] size_type count(Key const& k) const {
        std::shared_lock guard{rw_lock_};
        return storage_.count(::comdare::fingerprint::to_binary_string(k));
    }
    [[nodiscard]] bool contains(Key const& k) const {
        std::shared_lock guard{rw_lock_};
        return storage_.contains(::comdare::fingerprint::to_binary_string(k));
    }

    // ─── Read-Ops (natuerliche Returns) ──────────────────────────────────────
    [[nodiscard]] std::optional<mapped_type> lookup(Key const& k) const {
        auto bk = ::comdare::fingerprint::to_binary_string(k);
        std::shared_lock guard{rw_lock_};
        auto it = storage_.find(bk);
        if (it == storage_.end()) {
            ++const_misses_;
            return std::nullopt;
        }
        ++const_hits_;
        return it->second.second;
    }

    [[nodiscard]] std::optional<mapped_type> find(Key const& k) const {
        return lookup(k);
    }

    [[nodiscard]] std::optional<mapped_type> at(Key const& k) const {
        return lookup(k);
    }

    // ─── Iterator-Zugriff (read-only) ────────────────────────────────────────
    [[nodiscard]] const_iterator begin() const noexcept { return storage_.cbegin(); }
    [[nodiscard]] const_iterator end()   const noexcept { return storage_.cend(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return storage_.cbegin(); }
    [[nodiscard]] const_iterator cend()   const noexcept { return storage_.cend(); }
    [[nodiscard]] const_iterator lower_bound(Key const& k) const {
        std::shared_lock guard{rw_lock_};
        return storage_.lower_bound(::comdare::fingerprint::to_binary_string(k));
    }
    [[nodiscard]] const_iterator upper_bound(Key const& k) const {
        std::shared_lock guard{rw_lock_};
        return storage_.upper_bound(::comdare::fingerprint::to_binary_string(k));
    }
    [[nodiscard]] std::pair<const_iterator, const_iterator>
    equal_range(Key const& k) const {
        std::shared_lock guard{rw_lock_};
        return storage_.equal_range(::comdare::fingerprint::to_binary_string(k));
    }

    // Range-Scan (PRT-ART-spezifisch, REV 7 §4.2(f))
    [[nodiscard]] std::vector<mapped_type>
    range_scan(Key const& start, size_type max_count) const {
        auto bk = ::comdare::fingerprint::to_binary_string(start);
        std::shared_lock guard{rw_lock_};
        std::vector<mapped_type> result;
        result.reserve(max_count);
        auto it = storage_.lower_bound(bk);
        while (it != storage_.end() && result.size() < max_count) {
            result.push_back(it->second.second);
            ++it;
        }
        return result;
    }

    // ─── Schreib-Ops (returnen int errno-style) ─────────────────────────────
    // 2-Param: insert(Key, Value)
    template <std::size_t N = kValueArity>
    std::enable_if_t<N == 1, status_t>
    insert(Key const& k, First const& v) {
        return insert_internal(k, mapped_type(v));
    }

    // N>2: insert(Key, V1, V2, ...) - akzeptiert die einzelnen Values
    template <std::size_t N = kValueArity>
    std::enable_if_t<(N > 1), status_t>
    insert(Key const& k, First const& v1, Rest const&... vs) {
        return insert_internal(k, mapped_type{v1, vs...});
    }

    // Direkt mit fertigem mapped_type (auch fuer Tuple-Construct)
    status_t insert_mapped(Key const& k, mapped_type const& mv) {
        return insert_internal(k, mv);
    }

    status_t erase(Key const& k) {
        auto bk = ::comdare::fingerprint::to_binary_string(k);
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        auto it = storage_.find(bk);
        if (it == storage_.end()) return status_key_not_found;
        storage_.erase(it);
        ++erases_;
        return status_ok;
    }

    status_t clear() {
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        storage_.clear();
        return status_ok;
    }

    status_t set(Key const& k, mapped_type const& v) {
        auto bk = ::comdare::fingerprint::to_binary_string(k);
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        auto it = storage_.find(bk);
        if (it == storage_.end()) {
            storage_.try_emplace(bk, storage_value_t{k, v});
            ++inserts_;
            update_density(bk);
        } else {
            it->second.second = v;
        }
        return status_ok;
    }

    // ─── Counters / Stats (read-only) ────────────────────────────────────────
    [[nodiscard]] std::uint64_t total_inserts() const noexcept { return inserts_; }
    [[nodiscard]] std::uint64_t total_erases()  const noexcept { return erases_; }
    [[nodiscard]] std::uint64_t total_hits()    const noexcept { return const_hits_; }
    [[nodiscard]] std::uint64_t total_misses()  const noexcept { return const_misses_; }

    // ─── Identity API ─────────────────────────────────────────────────────────
    [[nodiscard]] static ::comdare::cache_engine::PermutationFlags identity_flags() noexcept {
        return prt_art_permutation_flags();
    }
    [[nodiscard]] static std::string identifier() {
        return prt_art_identifier();
    }

    // ─── Komponenten-Accessors ───────────────────────────────────────────────
    [[nodiscard]] auto& pools()              noexcept { return components_.pools; }
    [[nodiscard]] auto& concurrency()        noexcept { return components_.concurrency; }
    [[nodiscard]] auto& memory_layout()      noexcept { return components_.memory_layout; }
    [[nodiscard]] auto& path_prefetch()      noexcept { return components_.path_prefetch; }
    [[nodiscard]] auto& density_tracker()    noexcept { return components_.density_tracker; }
    [[nodiscard]] auto& hypothesis_metrics() noexcept { return components_.hypothesis_metrics; }

private:
    status_t insert_internal(Key const& k, mapped_type const& mv) {
        auto bk = ::comdare::fingerprint::to_binary_string(k);
        ::comdare::prt_art::concurrency::WriteGuard olc_guard{components_.concurrency};
        std::unique_lock lock{rw_lock_};
        try {
            auto [it, inserted] = storage_.try_emplace(bk, storage_value_t{k, mv});
            if (!inserted) {
                return status_key_already_exists;
            }
            ++inserts_;
            update_density(bk);
            return status_ok;
        } catch (std::bad_alloc const&) {
            return status_out_of_memory;
        }
    }

    void update_density(binary_key_t const& bk) {
        std::uint64_t const synthetic_node_id =
            static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(bk.data()) & 0xFFFFu);
        std::size_t const sz = storage_.size();
        double const density_pct = (sz * 100.0)
            / static_cast<double>((std::max<std::size_t>)(1, sz + 64));
        components_.density_tracker.record(synthetic_node_id, density_pct);
    }

    mutable std::shared_mutex rw_lock_;
    storage_map_t             storage_;
    PrtArtComponents          components_;

    std::uint64_t           inserts_      = 0;
    std::uint64_t           erases_       = 0;
    mutable std::uint64_t   const_hits_   = 0;
    mutable std::uint64_t   const_misses_ = 0;
};

}  // namespace comdare::prt_art::identity
