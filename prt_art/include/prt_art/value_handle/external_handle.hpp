#pragma once
// ExternalHandle (PRT-ART Pflicht-Datenpfad) — Pointer auf externen Payload-Speicher
// REV 6 §5.4 — fuer grosse Werte ueberhalb der InlineHandle-Capacity
//
// Speichert nur ein Handle (Pool-relative Position), nicht den Wert selbst.
// Der Pool wird vom Allocator verwaltet (siehe prt_art/allocator/).

#include <cstdint>

namespace comdare::prt_art::value_handle {

class ExternalHandle {
public:
    static constexpr std::uint64_t kInvalidHandle = static_cast<std::uint64_t>(-1);

    ExternalHandle() = default;

    explicit ExternalHandle(std::uint64_t pool_offset, std::uint32_t value_bytes)
        : pool_offset_(pool_offset), value_bytes_(value_bytes) {}

    [[nodiscard]] std::uint64_t pool_offset() const noexcept { return pool_offset_; }
    [[nodiscard]] std::uint32_t value_bytes() const noexcept { return value_bytes_; }

    [[nodiscard]] bool is_valid() const noexcept { return pool_offset_ != kInvalidHandle; }

    void invalidate() noexcept {
        pool_offset_ = kInvalidHandle;
        value_bytes_ = 0;
    }

private:
    std::uint64_t pool_offset_ = kInvalidHandle;
    std::uint32_t value_bytes_ = 0;
};

} // namespace comdare::prt_art::value_handle
