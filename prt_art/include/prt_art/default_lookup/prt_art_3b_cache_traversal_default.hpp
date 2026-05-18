#pragma once
// V32.FF.1 (2026-05-18 spaet) - Default-Lookup-Indikator fuer Achse 3.B
//
// @achse 3.B
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine/concepts/disciplines/array_discipline.hpp
//
// PRT-ART hat KEINE eigene Cache-Memory-Traversal-Implementation.
// CEB-AutoPermutator nutzt CE-Defaults:
//   - cache-line-walk (64B Cache-Line/Schritt)
//   - prefetch-stride
//   - page-jump (4KB)
//   - HBM-near-bank-walk (auf HBM-Hardware)
//   - cache-oblivious-recursion

namespace comdare::prt_art::default_lookup {
}  // namespace comdare::prt_art::default_lookup
