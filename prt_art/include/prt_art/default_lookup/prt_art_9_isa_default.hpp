#pragma once
// V32.FF.1 (2026-05-18 spaet) - Default-Lookup-Indikator fuer Achse 9 ISA
//
// @achse 9
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine IPlatformProbe-Output
//
// PRT-ART hat KEINE eigene ISA-Wahl. Wird per IPlatformProbe (Phase 1 DISCOVER)
// vom Host bestimmt:
//   - x86_64 mit AVX2 / AVX-512 / SSE-Sub-Variants
//   - ARM64 mit NEON / SVE / SVE2
//   - RISC-V (mit V-Extension auf moderner Hardware)
//   - ...

namespace comdare::prt_art::default_lookup {
}  // namespace comdare::prt_art::default_lookup
