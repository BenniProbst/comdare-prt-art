#pragma once
// LinearValueBuffer — append-only Buffer mit Lazy Deletion
// REV 6 §5.17 — Linear Value-Buffer mit Lazy Deletion
//
// Werte werden append-only geschrieben. Loeschungen markieren den Slot als
// tombstone, kein Move/Compaction. Periodische Garbage-Collection kann tombstone-
// Slots sammeln (Phase 7 — hier nur die Tracking-Logik).

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace comdare::prt_art::value_buffer {

struct ValueRecord {
    std::uint64_t offset       = 0;
    std::uint32_t bytes        = 0;
    bool          tombstone    = false;
};

class LinearValueBuffer {
public:
    // Schreibt payload an aktuelles Ende, liefert die Slot-ID zurueck
    [[nodiscard]] std::uint64_t append(std::span<std::byte const> payload) {
        ValueRecord r{};
        r.offset = static_cast<std::uint64_t>(buffer_.size());
        r.bytes  = static_cast<std::uint32_t>(payload.size());
        buffer_.insert(buffer_.end(), payload.begin(), payload.end());
        records_.push_back(r);
        return static_cast<std::uint64_t>(records_.size() - 1);
    }

    [[nodiscard]] std::span<std::byte const> read(std::uint64_t slot_id) const noexcept {
        if (slot_id >= records_.size()) return {};
        auto const& r = records_[slot_id];
        if (r.tombstone) return {};
        return std::span<std::byte const>(buffer_.data() + r.offset, r.bytes);
    }

    // Lazy Deletion: markiert nur, kein Move/Compaction
    void erase(std::uint64_t slot_id) noexcept {
        if (slot_id < records_.size() && !records_[slot_id].tombstone) {
            records_[slot_id].tombstone = true;
            ++tombstone_count_;
            tombstone_bytes_ += records_[slot_id].bytes;
        }
    }

    [[nodiscard]] bool is_alive(std::uint64_t slot_id) const noexcept {
        return slot_id < records_.size() && !records_[slot_id].tombstone;
    }

    [[nodiscard]] std::size_t total_records()  const noexcept { return records_.size(); }
    [[nodiscard]] std::size_t alive_records()  const noexcept { return records_.size() - tombstone_count_; }
    [[nodiscard]] std::size_t tombstone_records() const noexcept { return tombstone_count_; }
    [[nodiscard]] std::size_t buffer_bytes()   const noexcept { return buffer_.size(); }
    [[nodiscard]] std::size_t tombstone_bytes() const noexcept { return tombstone_bytes_; }

    // Gibt true zurueck wenn Garbage-Collection sich lohnt (Tombstone-Anteil > Schwellwert)
    [[nodiscard]] bool should_compact(double tombstone_threshold = 0.30) const noexcept {
        if (buffer_.empty()) return false;
        double share = static_cast<double>(tombstone_bytes_) / static_cast<double>(buffer_.size());
        return share >= tombstone_threshold;
    }

    // Compaction: konsolidiert lebende Werte in einem neuen Buffer (returns new slot mapping)
    [[nodiscard]] std::vector<std::uint64_t> compact() {
        std::vector<std::uint64_t> mapping(records_.size(), static_cast<std::uint64_t>(-1));
        std::vector<std::byte>     new_buffer;
        std::vector<ValueRecord>   new_records;
        new_buffer.reserve(buffer_.size() - tombstone_bytes_);
        new_records.reserve(alive_records());

        for (std::size_t i = 0; i < records_.size(); ++i) {
            auto const& r = records_[i];
            if (r.tombstone) continue;
            ValueRecord nr{};
            nr.offset = static_cast<std::uint64_t>(new_buffer.size());
            nr.bytes  = r.bytes;
            new_buffer.insert(new_buffer.end(),
                              buffer_.begin() + r.offset,
                              buffer_.begin() + r.offset + r.bytes);
            mapping[i] = static_cast<std::uint64_t>(new_records.size());
            new_records.push_back(nr);
        }

        buffer_         = std::move(new_buffer);
        records_        = std::move(new_records);
        tombstone_count_ = 0;
        tombstone_bytes_ = 0;
        return mapping;
    }

private:
    std::vector<std::byte>   buffer_{};
    std::vector<ValueRecord> records_{};
    std::size_t              tombstone_count_ = 0;
    std::size_t              tombstone_bytes_ = 0;
};

}  // namespace comdare::prt_art::value_buffer
