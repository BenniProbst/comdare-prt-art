# prt_art/include/prt_art/default_lookup/ — V32.DD.2 Default-Lookup-Indikatoren (AA.3)

**Stand:** 2026-05-18 (V32.DD.2 Skelett-Verzeichnis)
**Trigger:** User-Direktive AA.3 KRITISCHE KORREKTUR

## Zweck

PRT-ART hat NICHT fuer alle Achsen eine eigene Implementierung. Bei fehlender Spec
nutzt der CacheEngineBuilder die CE-Bibliothek per Auto-Permutation.

Dieses Verzeichnis enthaelt **Default-Lookup-Indikatoren** als Platzhalter-Files:
sie signalisieren via Doxygen-Tag `@reuse_status (default-lookup)`, dass PRT-ART
auf der betreffenden Achse KEINE eigene Implementierung hat — der CEB-AutoPermutator
loest die Achse via CE-Bibliothek-Permutation auf.

## Default-Lookup-Achsen fuer PRT-ART (Stand AA.4 §0.3)

| Achse | Default-Lookup-File | CE-Bibliothek-Pfad |
|---|---|---|
| 3.B Cache-Memory-Traversal | `prt_art_3b_cache_traversal_default.hpp` | cache-engine/concepts/disciplines/array_discipline.hpp |
| 6.2 Reclamation-Policy | `prt_art_62_reclamation_default.hpp` | cache-engine/reclamation/rcu_reclaim/ + concepts/mechanics/comdare_rcu_mechanic |
| 6.3 NUMA-Affinity | `prt_art_63_numa_default.hpp` | (cache-engine V32 NEU: concepts/numa_affinity.hpp) |
| 6.4 Huge-Page-Policy | `prt_art_64_huge_page_default.hpp` | cache-engine/allocators/portable_aligned_alloc.hpp (Teil-Aspekt) |
| 8.2 Locking-Mode | `prt_art_82_locking_default.hpp` | cache-engine/concepts/disciplines/memory_*_discipline.hpp |
| 9 ISA | `prt_art_9_isa_default.hpp` | cache-engine/IPlatformProbe Output |
| 11 TELEMETRY | `prt_art_11_telemetry_default.hpp` | cache-engine/concepts/telemetry/leaf_only_counter.hpp + sampled + retroactive + per_node |
| 12 HARDWARE-STRATEGY | `prt_art_12_hardware_default.hpp` | (cache-engine V32 NEU: concepts/hardware_strategy.hpp) |
| 13 SCHEDULING-STRATEGY | `prt_art_13_scheduling_default.hpp` | (cache-engine V32 NEU: concepts/scheduling_strategy.hpp) |

## Auto-Permutator-Workflow

1. CEB liest PrtArt-Profile-XML
2. Detect: Achse X nicht explizit definiert
3. Loop ueber Default-Lookup-Files (oder via Doxygen-Extraktion)
4. Pro Default-Lookup-Hinweis: lookup in CE-Bibliothek-Pfad
5. Generiere Permutationen (alle SOTA-Bausteine der Achse)
6. Pro Permutation: execute + measure
7. Result-Tabelle: best-of-axis = PRT-ART+X-Best-Variant

## V32.1+ Sprint TODO

Konkrete Platzhalter-Header-Files anlegen mit Doxygen-Tags + Verweisen auf CE-Bibliothek-Pfade.
