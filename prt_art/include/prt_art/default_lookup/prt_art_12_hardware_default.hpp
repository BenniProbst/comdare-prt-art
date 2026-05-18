#pragma once
// V32.DD.2 (2026-05-18) - Default-Lookup-Indikator fuer Achse 12 HARDWARE-STRATEGY
//
// @achse 12
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine/concepts/hardware_strategy.hpp (V32 NEU)
//
// PRT-ART hat KEINE eigene Hardware-Strategy-Implementation.
// CEB-AutoPermutator nutzt CE-Bibliothek zur Auto-Permutation aller verfuegbaren
// Hardware-Strategien:
//   - SIMD-Family: AVX2, AVX-512, NEON, SVE2, scalar
//   - Cache-Level-Targeting: L1/L2/L3/HBM-aware
//   - NUMA-Strategy: local, interleave, preferred, bind, none
//   - Prefetch-Hardware: PREFETCH, PREFETCHNTA, PREFETCHW
//   - Atomic-Family: CAS, LL-SC, RmwExtended
//
// Plattform-Filter per IPlatformProbe: nur Host-faehige Sub-Achsen-Auspraegungen
// werden permutiert. messreihen.xml kann via allowed_variants= weiter einschraenken.
//
// Wenn PRT-ART spaeter eine eigene Hardware-Strategy braucht: hier ueberschreiben
// mit @reuse_status (b) und konkrete Klassen-Definition.

namespace comdare::prt_art::default_lookup {
// Bewusst KEINE Klassen-Definition - Platzhalter signalisiert Auto-Permutation.
}  // namespace comdare::prt_art::default_lookup
