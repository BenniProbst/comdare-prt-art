#pragma once
// V32.DD.2 (2026-05-18) - Default-Lookup-Indikator fuer Achse 13 SCHEDULING-STRATEGY
//
// @achse 13
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine/concepts/scheduling_strategy.hpp (V32 NEU)
//
// PRT-ART hat KEINE eigene Scheduling-Strategy-Implementation.
// CEB-AutoPermutator nutzt CE-Bibliothek zur Auto-Permutation:
//   - Worker-Pool-Layout: thread-per-core, work-stealing, CPU-pinning
//   - SIMD-Worker-Count-Limit: hardware-limitiert (typ. 2 von N Cores)
//   - Hetero-Core-Dispatch: Intel Hybrid P/E
//   - Co-Routine-Strategy: interleave, dependency-tracking
//   - Batch-Granularity: single, micro-batch, macro-batch

namespace comdare::prt_art::default_lookup {
// Bewusst KEINE Klassen-Definition - Platzhalter signalisiert Auto-Permutation.
}  // namespace comdare::prt_art::default_lookup
