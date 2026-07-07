#pragma once
// ═══ QUARANTAENE (AP-2-neu/#236 W4, 2026-07-07) ═══ ALT-PFAD — NICHT Teil des Mess-Pfads!
// Der EINZIGE PRT-ART-Mess-Pfad ist der cache-engine-KATALOG-PFAD (sota_catalog -> echte
// PrtArtComposition als DLL; ce-Gegenbeweis: test_ap2_katalog_pfad_stubfrei). Diese Datei
// gehoert zu einem der drei Alt-Pfade (V18-Template / EE-A-B-Vergleich / Registry-run) und
// traegt bewusst Stub-/Surrogat-Code — sie darf NIE in den Mess-Pfad verdrahtet werden.
// V32.FF.2 (2026-05-18) + V34.A.2 (2026-05-21) - PrtArt als ExecutionEngine B (AA.2)
//
// @subsystem PA
// @reuse_status (b)
//
// AA.2-Korrektur: PrtArt ist EIGENSTAENDIGE ExecutionEngine (nicht "Pruefling im
// CE-Subsystem"). Dieser Adapter macht PrtArtSearchEngine ueber IExecutingEngine
// fuer den CacheEngineBuilder ansprechbar - gleichwertig zur CE-as-EE-A.
//
// V34.A.2: Backend-Template + as_engine_callable() Bridge zu ExecuteEngineCommand
// (V33.A.1 DI). Default-Backend = std::unordered_map (= "innovative" Variante mit
// O(1) statt O(log n) wie CE-EE-A std::map).

#include "cache_engine/builder/commands/execute_engine_command.hpp"
// Note: include-Pfad wird via CMake gesetzt
//   target_include_directories(... PRIVATE
//     ${COMDARE_CACHE_ENGINE_DIR}/libs)

#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace comdare::prt_art::identity {

namespace cmd = comdare::cache_engine::builder::commands;

/**
 * @brief PrtArtHashBackend - Default-Backend (std::unordered_map) als PRT-ART-Surrogat
 * @subsystem PA
 *
 * In V34.A.2 ein In-Memory unordered_map als Stand-in fuer die eigentliche
 * PrtArtSearchEngine. Im realen V35+ Sprint wird PrtArtSearchEngine direkt verwendet.
 */
template <typename Key = std::string, typename Value = std::uint64_t>
class PrtArtHashBackend {
public:
    // Concurrency-Safety per Konvention: pro EE-Instanz eigenes Backend.

    [[nodiscard]] std::optional<Value> lookup(const Key& key) const {
        auto it = store_.find(key);
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }

    bool insert(const Key& key, const Value& value) {
        auto [it, ok] = store_.try_emplace(key, value);
        if (!ok) it->second = value;
        return ok;
    }

    [[nodiscard]] std::size_t size() const { return store_.size(); }

private:
    std::unordered_map<Key, Value> store_;
};

/**
 * @brief PrtArtExecutionEngineAdapter - macht PrtArt zur ExecutionEngine B
 * @subsystem PA
 * @reuse_status (b)
 *
 * Generischer Backend-Adapter. Default-Backend ist PrtArtHashBackend (O(1)).
 *
 * Vergleich zur CacheEngineExecutionEngineAdapter (EE-A):
 *   EE-A = CacheEngine pure SOTA-Baseline (std::map, O(log n))
 *   EE-B = PrtArt mit Innovationen (std::unordered_map, O(1)) - im V35+ wird das
 *          ein echter PrtArtSearchEngine-Template mit 14 Achsen-Konfiguration
 */
template <typename Backend = PrtArtHashBackend<std::string, std::uint64_t>>
class PrtArtExecutionEngineAdapter {
public:
    using key_type   = std::string;
    using value_type = std::uint64_t;

    PrtArtExecutionEngineAdapter() = default;
    explicit PrtArtExecutionEngineAdapter(Backend backend) noexcept : backend_{std::move(backend)} {}

    [[nodiscard]] static constexpr std::string_view engine_name() noexcept { return "PrtArt-EE-B"; }

    /// V32-API: execute(workload)
    [[nodiscard]] cmd::ExecutionResult execute(const cmd::Workload& workload) {
        cmd::ExecutionResult result{};
        result.engine_name   = engine_name();
        result.workload_kind = workload.kind;
        result.success       = true;
        return result;
    }

    /// V34.A.2: EngineCallable fuer DI in ExecuteEngineCommand
    [[nodiscard]] cmd::EngineCallable as_engine_callable() {
        return [this](std::size_t op, cmd::WorkloadKind kind, std::uint64_t seed) {
            cmd::OperationOutcome outcome{};
            const auto            key = make_key(op, seed);
            switch (kind) {
                case cmd::WorkloadKind::YCSB_C_ReadOnly: {
                    auto val                   = this->backend_.lookup(key);
                    outcome.cache_misses_delta = val.has_value() ? 0u : 1u;
                    outcome.bytes_touched      = 24u; // PRT-ART kompakter dank Inline-Value
                    outcome.success            = true;
                    break;
                }
                case cmd::WorkloadKind::YCSB_A_Read50Write50:
                case cmd::WorkloadKind::YCSB_F_ReadModifyWrite: {
                    const bool do_write = ((op + seed) % 2u == 0u);
                    if (do_write) {
                        const value_type v    = static_cast<value_type>(op);
                        outcome.success       = this->backend_.insert(key, v);
                        outcome.bytes_touched = 48u;
                    } else {
                        auto val                   = this->backend_.lookup(key);
                        outcome.cache_misses_delta = val.has_value() ? 0u : 1u;
                        outcome.bytes_touched      = 24u;
                        outcome.success            = true;
                    }
                    break;
                }
                default: {
                    const value_type v = static_cast<value_type>(op);
                    this->backend_.insert(key, v);
                    outcome.bytes_touched = 48u;
                    outcome.success       = true;
                    break;
                }
            }
            ops_executed_.fetch_add(1, std::memory_order_relaxed);
            return outcome;
        };
    }

    [[nodiscard]] std::optional<value_type> lookup(const key_type& key) const { return backend_.lookup(key); }

    bool insert(const key_type& key, value_type value) { return backend_.insert(key, value); }

    [[nodiscard]] Backend&       backend() noexcept { return backend_; }
    [[nodiscard]] const Backend& backend() const noexcept { return backend_; }

    [[nodiscard]] std::uint64_t ops_executed() const noexcept { return ops_executed_.load(std::memory_order_relaxed); }

private:
    static key_type make_key(std::size_t op, std::uint64_t seed) {
        return "k" + std::to_string(seed) + "_" + std::to_string(op);
    }

    Backend                    backend_{};
    std::atomic<std::uint64_t> ops_executed_{0};
};

} // namespace comdare::prt_art::identity
