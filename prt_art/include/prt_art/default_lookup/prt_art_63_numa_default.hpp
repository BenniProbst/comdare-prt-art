#pragma once
// V32.FF.1 (2026-05-18 spaet) - Default-Lookup-Indikator fuer Achse 6.3
//
// @achse 6.3
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine/concepts/numa_affinity.hpp (V32.EE.5 NEU)
//
// PRT-ART hat KEINE eigene NUMA-Affinity-Strategie.
// CEB-AutoPermutator nutzt CE-NumaStrategy-Enum:
//   - Local (Standard)
//   - Interleave
//   - Preferred
//   - Bind

namespace comdare::prt_art::default_lookup {
}  // namespace comdare::prt_art::default_lookup
