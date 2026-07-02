#pragma once
// InlineHandle (PRT-ART Pflicht-Datenpfad) — kleine Werte direkt im terminalen Knoten
// REV 6 §5.4 — H3-Hypothese: Inline-vs-External-Grenze ueber Kostenmodell waehlen
//
// Speichert Werte bis kInlineCapacity Bytes direkt inline (typisch ≤8 Byte).
// Vermeidet Pointer-Indirection, profitiert von gleicher Cache-Line wie Knoten-Header.

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

namespace comdare::prt_art::value_handle {

template <std::size_t Capacity = 8>
class InlineHandle {
    static_assert(Capacity > 0 && Capacity <= 64, "InlineHandle Capacity muss 1..64 sein");

public:
    static constexpr std::size_t kInlineCapacity = Capacity;

    InlineHandle() = default;

    explicit InlineHandle(std::span<std::byte const> bytes) { store(bytes); }

    void store(std::span<std::byte const> bytes) noexcept {
        std::size_t n = bytes.size() <= Capacity ? bytes.size() : Capacity;
        for (std::size_t i = 0; i < n; ++i) data_[i] = bytes[i];
        size_ = static_cast<std::uint8_t>(n);
    }

    [[nodiscard]] std::span<std::byte const> bytes() const noexcept {
        return std::span<std::byte const>(data_.data(), size_);
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool        empty() const noexcept { return size_ == 0; }

    [[nodiscard]] static constexpr bool fits(std::size_t value_bytes) noexcept { return value_bytes <= Capacity; }

private:
    std::array<std::byte, Capacity> data_{};
    std::uint8_t                    size_ = 0;
};

} // namespace comdare::prt_art::value_handle
