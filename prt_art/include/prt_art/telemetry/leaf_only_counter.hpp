#pragma once
// V34.E.1 (2026-05-21) - PRT-ART eigene LeafOnlyCounter-Implementation (Achse 11.X1)
//
// @achse 11
// @subsystem PA
// @phase_owner PA
// @kuehn_variant LeafOnlyCounter (Hauptvariante 2024+)
// @reuse_status (a)
//
// Kuehn-validierte Telemetrie-Strategie (Mail 2026-05-08, Code-Repo 2026-05-21):
// "Insbesondere auf die Knoten in den oberen Ebenen des Baums wird sehr haeufig
//  zugegriffen, sodass diese typischerweise im Caches mehrerer Kerne liegen.
//  Zusaetzliche Schreibzugriffe fuehren dann durch die Cache-Kohaerenzmechanismen
//  schnell zu starkem Cacheline-Ping-Pong zwischen den Kernen, was die Performance
//  entsprechend beeintraechtigt."
//
// Loesung: Counter NUR in Blatt-Knoten (Leaf-Only), nie in Inner-Nodes.
//
// Vergleich mit PerNodeCounter (siehe unten) als ANTI-PATTERN dokumentiert.
//
// Code-Referenz: Forschungsarbeiten/code/P28-Kuehn-DAMON/bplustree_reordering/
//   include/btree_olc.hpp (Kuehn's Original-Impl in C++17).

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace comdare::prt_art::telemetry {

/**
 * @brief NodeAccessCount - Atomic-Counter pro Blatt-Knoten
 * @achse 11
 * @subsystem PA
 *
 * Verwendet std::atomic mit memory_order_relaxed (per Kuehn-Empfehlung).
 * Cache-Line-Padding optional — wenn jedes Counter eigene Cache-Line bekommt,
 * kann False-Sharing zwischen Counter-Tupeln vermieden werden.
 */
struct alignas(64) NodeAccessCount {
    std::atomic<std::uint64_t> counter {0};
    char padding[64 - sizeof(std::atomic<std::uint64_t>)] {};
};

/**
 * @brief LeafOnlyCounter - Kuehn 11.X1 Hauptvariante (V34.E.1)
 * @achse 11
 * @subsystem PA
 * @kuehn_variant LeafOnlyCounter
 *
 * Eigenstaendige PRT-ART-Implementation der Kuehn-Telemetrie-Hauptvariante.
 * Counter werden NUR in Blatt-Knoten gehalten, nie in Inner-Nodes.
 *
 * Konsequenz: Cache-Line-Ping-Pong zwischen Kernen wird vermieden, weil
 * Inner-Nodes (die haeufig von mehreren Kernen gelesen werden) keine
 * Schreib-Operationen mehr erfahren.
 *
 * Template-Param NodeId: Typ des Knoten-Identifiers (Default = void*).
 * Im Diplomarbeit-Setup wird NodeId = std::uint64_t (logischer Offset).
 *
 * Beispiel:
 *   LeafOnlyCounter<std::uint64_t> tel;
 *   tel.record_access(0x1000, true);   // Leaf -> counter++
 *   tel.record_access(0x2000, false);  // Inner -> no-op (kein Pingpong)
 *   auto count = tel.access_count(0x1000);  // 1
 */
template <typename NodeId = void*>
class LeafOnlyCounter {
public:
    /// Hauptmethode: pro Zugriff aufrufen. is_leaf entscheidet ob Counter inkrementiert wird.
    void record_access(NodeId node, bool is_leaf) {
        if (!is_leaf) {
            // KRITISCH: Inner-Nodes NIE schreiben (vermeidet Cache-Line-Ping-Pong, Kuehn 2023)
            return;
        }
        auto& slot = ensure_slot(node);
        slot.counter.fetch_add(1, std::memory_order_relaxed);
    }

    /// Liest den Counter eines Blatt-Knotens (0 wenn nie zugegriffen)
    [[nodiscard]] std::uint64_t access_count(NodeId node) const {
        auto it = counts_.find(node);
        if (it == counts_.end()) return 0;
        return it->second->counter.load(std::memory_order_relaxed);
    }

    /// Anzahl tracked Blatt-Knoten
    [[nodiscard]] std::size_t tracked_leaves() const noexcept {
        return counts_.size();
    }

    /// Cache-Pressure-Estimate: normalisiertes Mass [0.0, 1.0]
    /// = (Anzahl Leaves mit count > 0) / (Anzahl tracked Leaves)
    [[nodiscard]] double cache_pressure_estimate() const {
        if (counts_.empty()) return 0.0;
        std::size_t hot = 0;
        for (const auto& [_, slot] : counts_) {
            if (slot->counter.load(std::memory_order_relaxed) > 0) ++hot;
        }
        return static_cast<double>(hot) / static_cast<double>(counts_.size());
    }

    /// Aggregate-Counter ueber alle tracked Leaves
    [[nodiscard]] std::uint64_t total_accesses() const {
        std::uint64_t sum = 0;
        for (const auto& [_, slot] : counts_) {
            sum += slot->counter.load(std::memory_order_relaxed);
        }
        return sum;
    }

    /// Reset (vor naechster Mess-Phase)
    void reset() {
        counts_.clear();
    }

private:
    NodeAccessCount& ensure_slot(NodeId node) {
        auto it = counts_.find(node);
        if (it == counts_.end()) {
            auto slot = std::make_unique<NodeAccessCount>();
            auto& ref = *slot;
            counts_.emplace(node, std::move(slot));
            return ref;
        }
        return *(it->second);
    }

    // unique_ptr weil NodeAccessCount nicht-movable (atomic ist nicht-movable)
    std::unordered_map<NodeId, std::unique_ptr<NodeAccessCount>> counts_;
};

/**
 * @brief PerNodeCounter (Kuehn 11.X4) - ANTI-PATTERN
 * @achse 11
 * @subsystem PA
 * @kuehn_variant PerNodeCounter
 * @anti_pattern Cache-Line-Ping-Pong zwischen Kernen!
 *
 * Counter in ALLEN Knoten (auch Inner-Nodes). Wird in der Diplomarbeit
 * als Vergleichsbasis verwendet um Kuehn's Behauptung quantitativ zu validieren:
 * LeafOnlyCounter ist signifikant schneller als PerNodeCounter unter
 * Multi-Reader-Workload.
 *
 * Diese Klasse ist BEWUSST verfuegbar fuer F15-Vergleich (CompareEngineCommand
 * mit Welch-T-Test). Sie soll im Produktiv-Pfad NICHT verwendet werden.
 */
template <typename NodeId = void*>
class PerNodeCounter {
public:
    /// Schreibt IMMER, egal ob Leaf oder Inner. Cache-Line-Ping-Pong-Risiko.
    void record_access(NodeId node, bool /*is_leaf*/) {
        auto& slot = ensure_slot(node);
        slot.counter.fetch_add(1, std::memory_order_relaxed);
    }

    [[nodiscard]] std::uint64_t access_count(NodeId node) const {
        auto it = counts_.find(node);
        if (it == counts_.end()) return 0;
        return it->second->counter.load(std::memory_order_relaxed);
    }

    [[nodiscard]] std::size_t tracked_nodes() const noexcept {
        return counts_.size();
    }

    [[nodiscard]] std::uint64_t total_accesses() const {
        std::uint64_t sum = 0;
        for (const auto& [_, slot] : counts_) {
            sum += slot->counter.load(std::memory_order_relaxed);
        }
        return sum;
    }

    void reset() { counts_.clear(); }

private:
    NodeAccessCount& ensure_slot(NodeId node) {
        auto it = counts_.find(node);
        if (it == counts_.end()) {
            auto slot = std::make_unique<NodeAccessCount>();
            auto& ref = *slot;
            counts_.emplace(node, std::move(slot));
            return ref;
        }
        return *(it->second);
    }

    std::unordered_map<NodeId, std::unique_ptr<NodeAccessCount>> counts_;
};

}  // namespace comdare::prt_art::telemetry
