#pragma once
// BytePath — Schluessel-Encoding fuer den TLB-Offset-Adressierungs-Stack
// REV 6 §5.17
//
// Wraps eine Byte-Sequenz mit Cursor-Pointer + Hilfsfunktionen zum Konsumieren
// von 1..N Byte-Praefixen fuer hierarchische VirtualOffsetAddress-Berechnung.

#include <cstddef>
#include <cstdint>
#include <span>

namespace comdare::prt_art::memory_layout {

class BytePath {
public:
    BytePath() = default;
    explicit BytePath(std::span<std::byte const> key) : key_(key) {}

    [[nodiscard]] std::size_t cursor() const noexcept { return cursor_; }
    [[nodiscard]] std::size_t total_bytes() const noexcept { return key_.size(); }
    [[nodiscard]] std::size_t remaining() const noexcept { return key_.size() - cursor_; }
    [[nodiscard]] bool        exhausted() const noexcept { return cursor_ >= key_.size(); }

    [[nodiscard]] std::span<std::byte const> peek(std::size_t n) const noexcept {
        std::size_t avail = remaining();
        std::size_t take  = n <= avail ? n : avail;
        return key_.subspan(cursor_, take);
    }

    void advance(std::size_t n) noexcept { cursor_ += (n <= remaining()) ? n : remaining(); }

    void reset() noexcept { cursor_ = 0; }

    // Konsumiert die naechsten n Bytes und liefert sie zurueck (cursor wandert mit).
    [[nodiscard]] std::span<std::byte const> consume(std::size_t n) noexcept {
        auto s = peek(n);
        advance(s.size());
        return s;
    }

private:
    std::span<std::byte const> key_{};
    std::size_t                cursor_ = 0;
};

} // namespace comdare::prt_art::memory_layout
