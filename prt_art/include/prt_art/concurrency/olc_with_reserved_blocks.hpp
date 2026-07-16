#pragma once
// OlcWithReservedValueBlocks — Multi-Reader-Writer Concurrency (REV 6 §5.17)
//
// Kombiniert klassisches OLC (Optimistic Lock Coupling) mit reservierten Value-Bloecken:
//
//  Reader-Pfad: read_version → traverse → re-check_version → falls geaendert: restart
//  Writer-Pfad: increment_version (mark write) → reserve_value_block → schreibe in
//               exklusivem Block → increment_version (commit)
//
// Reservierte Value-Bloecke vermeiden, dass MVCC-Versionen denselben Cache-Line teilen
// (verhindert Cache-Coherence-Storm). Jeder Schreiber bekommt einen exklusiven Block
// (typisch 64-Byte cache-line aligned).

#include <atomic>
#include <cstdint>
#include <thread>

namespace comdare::prt_art::concurrency {

class OlcWithReservedValueBlocks {
public:
    using Version = std::uint64_t;

    static constexpr Version kWriteMarker   = 1; // niedrigstes Bit = Write-in-Progress
    static constexpr Version kIncrementStep = 2; // Increments in 2er-Schritten

    [[nodiscard]] Version read_version() const noexcept { return version_.load(std::memory_order_acquire); }

    [[nodiscard]] static constexpr bool is_write_in_progress(Version v) noexcept { return (v & kWriteMarker) != 0; }

    // Optimistic-Read: traverse mit captured_version, dann am Ende prueft validate()
    [[nodiscard]] bool validate(Version captured) const noexcept {
        Version current = read_version();
        return current == captured && !is_write_in_progress(current);
    }

    // Writer markiert Versions-Slot.
    // (REV-CXX-02, WP-5 2026-07-16): CAS-Schleife von GERADER auf die naechste UNGERADE Version statt
    // blindem fetch_add. Zwei gleichzeitige fetch_add-Writer kippten die Version von ungerade zurueck auf
    // gerade — ein optimistischer Leser konnte waehrend aktiver Writer scheinbar "kein Write" sehen und
    // inkonsistente Daten akzeptieren (Odd/Even-Vertrag verletzt). Jetzt gilt die Invariante: solange
    // IRGENDEIN Writer aktiv ist, ist die Version ungerade; ein zweiter Writer wartet (spin + yield), bis
    // end_write die naechste gerade Generation publiziert hat. Versionen bleiben monoton (+2 je Write);
    // Wraparound bei 2^64 ist theoretisch (>10^19 Writes) und erhaelt die Odd/Even-Paritaet.
    void begin_write() noexcept {
        Version expected = version_.load(std::memory_order_relaxed);
        for (;;) {
            if (is_write_in_progress(expected)) {
                std::this_thread::yield();
                expected = version_.load(std::memory_order_relaxed);
                continue;
            }
            if (version_.compare_exchange_weak(expected, expected + kWriteMarker, std::memory_order_acq_rel,
                                               std::memory_order_relaxed))
                return;
        }
    }

    void end_write() noexcept {
        // Increment um kIncrementStep + Loeschen des Write-Markers
        version_.fetch_add(kIncrementStep - kWriteMarker, std::memory_order_acq_rel);
    }

    // Reservierte Value-Blocks: Writer holt sich exklusiven Block-Index
    [[nodiscard]] std::uint64_t reserve_value_block() noexcept {
        return next_block_id_.fetch_add(1, std::memory_order_acq_rel);
    }

    [[nodiscard]] std::uint64_t total_blocks_reserved() const noexcept {
        return next_block_id_.load(std::memory_order_relaxed);
    }

    void reset() noexcept {
        version_.store(0, std::memory_order_release);
        next_block_id_.store(0, std::memory_order_release);
    }

private:
    std::atomic<Version>       version_{0};
    std::atomic<std::uint64_t> next_block_id_{0};
};

// RAII Writer-Guard
class WriteGuard {
public:
    explicit WriteGuard(OlcWithReservedValueBlocks& lock) : lock_(lock) {
        lock_.begin_write();
        block_id_ = lock_.reserve_value_block();
    }
    ~WriteGuard() { lock_.end_write(); }

    WriteGuard(WriteGuard const&)            = delete;
    WriteGuard& operator=(WriteGuard const&) = delete;

    [[nodiscard]] std::uint64_t reserved_block_id() const noexcept { return block_id_; }

private:
    OlcWithReservedValueBlocks& lock_;
    std::uint64_t               block_id_ = 0;
};

} // namespace comdare::prt_art::concurrency
