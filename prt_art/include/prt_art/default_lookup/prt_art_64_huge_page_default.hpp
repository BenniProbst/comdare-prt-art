#pragma once
// V32.FF.1 (2026-05-18 spaet) - Default-Lookup-Indikator fuer Achse 6.4
//
// @achse 6.4
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine/allocators/portable_aligned_alloc.hpp + Linux THP
//
// PRT-ART hat KEINE eigene Huge-Page-Policy.
// CEB-AutoPermutator nutzt CE-Bibliothek-Auspraegungen:
//   - transparent (THP, Linux-Default)
//   - explicit (madvise / VirtualAlloc Windows)
//   - none

namespace comdare::prt_art::default_lookup {
}  // namespace comdare::prt_art::default_lookup
