#pragma once
// V32.FF.1 (2026-05-18 spaet) - Default-Lookup-Indikator fuer Achse 8.2
//
// @achse 8.2
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine/concepts/locking_mode.hpp (V32.EE.5 NEU)
//
// PRT-ART OlcWithReservedBlocks nutzt mixed Locking, aber 8.2 Default-Permutation
// wird vom CEB durchlaufen:
//   - read-only (shared_mutex)
//   - read-write (exclusive_mutex)
//   - upgradeable (boost::shared_mutex upgrade)
//   - optimistic-validation (OLC, HTM)

namespace comdare::prt_art::default_lookup {
}  // namespace comdare::prt_art::default_lookup
