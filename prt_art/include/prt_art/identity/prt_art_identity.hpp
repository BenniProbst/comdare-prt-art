#pragma once
// PrtArtIdentity — PermutationFlags-Identitaet der PRT-ART
// REV 6 Final — bestimmte Konfiguration aus CacheEngine-Bausteinen
//
// Definiert die spezifische Bit-Kombination, die "PRT-ART" identifiziert.
// Wird vom CacheEngineBuilder gelesen, um das PRT-ART-Binary zu kompilieren.

#include <cache_engine/concepts/permutation_flags.hpp>

namespace comdare::prt_art::identity {

// Erzeugt PermutationFlags fuer die PRT-ART-Identitaet
[[nodiscard]] inline ::comdare::cache_engine::PermutationFlags prt_art_permutation_flags() noexcept {
    using namespace ::comdare::cache_engine::flags;
    ::comdare::cache_engine::PermutationFlags f{};

    // Page-Bank: kombiniert Redirect (CoCo) + DenseByte (ART) + Sparse (HOT)
    f.page_bank = page_bank::PRTART_REDIRECT
                | page_bank::PRTART_DENSEBYTE
                | page_bank::PRTART_SPARSEPATRICIA
                | page_bank::PRTART_EXTENDEDDENSE
                | page_bank::PRTART_CUSTOMCACHE;

    // Node-Bank: 2 PRT-ART Node-Typen
    f.node_bank = node_bank::PRTART_REDIRECT
                | node_bank::PRTART_BPLUS;

    // Traversal: PRT-ART eigene Hot-Path + Prefetch-Aware
    f.traversal_bank = traversal_bank::PRTART_HOT_PATH
                     | traversal_bank::PRTART_PREFETCH_AWARE;

    // ValueHandle: alle 3 Auspraegungen (Inline + Pointer + ChainRef)
    f.value_handle_bank = value_handle_bank::INLINE_HANDLE
                        | value_handle_bank::POINTER_HANDLE
                        | value_handle_bank::CHAINREF_HANDLE;

    // Memory-Layout: cache-line aligned + metadata header (TLB-Offset benutzt diese Eigenschaften)
    f.memory_layout_bank = memory_layout_bank::CACHELINE_ALIGNED
                         | memory_layout_bank::METADATA_HEADER;

    // Allocator: Pool-per-PageType + Arena-per-Subtree (bildet 4+2-Pool-Struktur ab)
    f.allocator_bank = allocator_bank::POOL_PER_PAGETYPE
                     | allocator_bank::ARENA_PER_SUBTREE;

    // Prefetch: PRT-ART-eigene Hot-Path + Adaptive-Distance
    f.prefetch_bank = prefetch_bank::HOT_PATH
                    | prefetch_bank::ADAPTIVE_DISTANCE;

    // Concurrency: OLC (mit reservierten Value-Bloecken)
    f.concurrency_bank = concurrency_bank::OLC;

    // ISA: Plattform-agnostisch (wird zur Compile-Time aus Plattform-Detection eingespielt)
    f.isa_bank = isa_bank::SCALAR_X86_64;   // Platzhalter — wird vom Builder ueberschrieben

    // Telemetry: Achse 11 — Path-Read + Probability-Hints (PRT-ART-typisch)
    f.telemetry_bank = telemetry_bank::PATH_READ_COUNTER
                     | telemetry_bank::PROBABILITY_HINTS;

    return f;
}

[[nodiscard]] inline std::string prt_art_identifier() {
    return prt_art_permutation_flags().to_identifier();
}

}  // namespace comdare::prt_art::identity
