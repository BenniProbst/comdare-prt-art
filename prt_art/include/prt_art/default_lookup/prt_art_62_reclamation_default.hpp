#pragma once
// V32.FF.1 (2026-05-18 spaet) - Default-Lookup-Indikator fuer Achse 6.2
//
// @achse 6.2
// @subsystem PA
// @reuse_status (default-lookup)
// @default_lookup_provided_by cache-engine/reclamation/rcu_reclaim/ + concepts/mechanics/comdare_rcu_mechanic.hpp
//
// PRT-ART hat KEINE eigene Reclamation-Policy.
// CEB-AutoPermutator nutzt CE-Bibliothek:
//   - epoch-based (Comdare-eigen RCU)
//   - RCU
//   - hazard-pointer (P30 Michael)
//   - QSBR (Quiescent-State-Based Reclamation)
//   - mark-sweep

namespace comdare::prt_art::default_lookup {
}  // namespace comdare::prt_art::default_lookup
