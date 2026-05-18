#pragma once
// V32.DD.2 (2026-05-18) - Achse 3.A SearchAlgo-Traversal (PRT-ART (b) Neu-Impl)
//
// @achse 3.A
// @subsystem PA
// @reuse_status (b)
//
// Reorganisiert aus prt_art/internal_search/ (Array256, Array65535, VectorU8U8, VectorU16U16).
// V31.F-Stand bleibt unter internal_search/ (Memory-Direktive); dies ist die V32-Sicht.

#include "../internal_search/array_256.hpp"
#include "../internal_search/array_65535.hpp"
#include "../internal_search/vector_u8_u8.hpp"
#include "../internal_search/vector_u16_u16.hpp"

namespace comdare::prt_art::traversal {

/**
 * @brief SearchAlgoTraversal - Achse 3.A Sub-Achsen-Aggregator
 * @achse 3.A
 * @subsystem PA
 * @reuse_status (b)
 *
 * PRT-ART hat 4 interne Suchtypen mit Dichte-Schwellen-Transition:
 * - Typ A (Array256): < 25% Dichte
 * - Typ B (Array65535): 25-50% Dichte
 * - Typ C (VectorU8U8): 50-75% Dichte, Range-Scan
 * - Typ D (VectorU16U16): > 75% Dichte, Range-Scan unsigned short
 */
template <typename Key>
class SearchAlgoTraversal {
public:
    // V32.DD.2 Skelett - Bridge zu V31-internal_search/-Klassen
    // V32.1 Sprint: konkrete Traversal-Dispatch-Logik
};

}  // namespace comdare::prt_art::traversal
