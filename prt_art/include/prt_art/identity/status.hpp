#pragma once
// status.hpp - errno-style Status-Codes fuer alle Schreib-/IO-Operationen
// REV 7 (2026-05-13): hybride SearchEngine-API (User-Direktive)
//
// Alle Schreiboperatoren (insert, erase, push_back, pop_back, clear, resize,
// reserve, assign, ...) returnen IMMER `int`:
//   0      = success
//   > 0    = spezifischer Fehlercode (siehe Konstanten unten)
//   < 0    = nicht verwendet (reserviert fuer extension)
//
// Leseoperatoren behalten ihren natuerlichen Returntyp (optional/T&/size_t).

namespace comdare::prt_art {

using status_t = int;

// Status-Code-Konstanten (errno-Style)
inline constexpr status_t status_ok                      = 0;
inline constexpr status_t status_key_already_exists      = 1;
inline constexpr status_t status_key_not_found           = 2;
inline constexpr status_t status_out_of_memory           = 3;
inline constexpr status_t status_invalid_argument        = 4;
inline constexpr status_t status_capacity_exceeded       = 5;
inline constexpr status_t status_locked                  = 6;
inline constexpr status_t status_out_of_range            = 7;
inline constexpr status_t status_empty_container         = 8;
inline constexpr status_t status_concurrent_modification = 9;
inline constexpr status_t status_io_error                = 10;

[[nodiscard]] inline constexpr bool status_is_ok(status_t s) noexcept { return s == 0; }
[[nodiscard]] inline constexpr bool status_is_error(status_t s) noexcept { return s != 0; }

} // namespace comdare::prt_art
