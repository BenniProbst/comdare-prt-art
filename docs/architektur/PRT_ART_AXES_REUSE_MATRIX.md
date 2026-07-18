# PRT-ART Achsen Reuse-Matrix (V32.FF.3 Spec)

**Stand:** 2026-05-18 (V32.FF.3)
**Quelle:** O.4 + AA.4 §0.3 Korrigierte Status-Markierungen
**Pflicht-Pre-Read:** `prt_art/include/prt_art/default_lookup/README.md`

## Status-Legende

| Status | Bedeutung |
|---|---|
| **(a)** | Reuse: PRT-ART pinnt SOTA-Baustein aus CE-Bibliothek explizit |
| **(b)** | Neu-Impl: PRT-ART bringt eigene Implementation (Forschungs-Beitrag) |
| **(c)** | Config: PRT-ART konfiguriert CE-Baustein mit spezifischer Permutation |
| **(default-lookup)** | CEB-AutoPermutator nutzt CE-Bibliothek (kein PrtArt-Code) |

## Achsen-Matrix

| Achse | PRT-ART-Klasse | Status | CE-Bibliothek-Verweis |
|---|---|---|---|
| 1 PAGE-TYPE | `nodes/redirect_node.hpp` + `nodes/bplus_node.hpp` | **(b)** Neu-Impl | (eigene Trie-Familie) |
| 2 NODE-TYPE | `nodes/bplus_node.hpp` mit 4 Suchtypen | **(b)** Neu-Impl | (eigene Sub-Variant-Familie A/B/C/D) |
| 3.A SearchAlgo-Traversal | `internal_search/` + `traversal/search_algo_traversal.hpp` | **(b)** Neu-Impl | (eigene byte-by-byte + Density-Schwellen) |
| 3.B Cache-Memory-Traversal | `default_lookup/prt_art_3b_cache_traversal_default.hpp` | **(default-lookup)** | cache-engine/concepts/disciplines/array_discipline.hpp |
| 3.M Traversal-Mapping | `traversal/traversal_mapping.hpp` (VirtualOffsetCalculator) | **(b)** Neu-Impl | (eigenes Mapping mit Pool-Slot-IDs) |
| 4 VALUEHANDLE | `value_handle/` (5 Header) | **(a)** Reuse K05 | (Konzept-Inline/External/ChainRef) |
| 5 MEMORY-LAYOUT | `memory_layout/cache_line_aligned_layout.hpp` | **(b)** Neu-Impl | (TLB-Offset + Cache-Line-aligned) |
| 6.1 Allocation-Strategy | `allocator/pool_router.hpp` + `pool_set.hpp` | **(b)** Neu-Impl | (4+2 Pool-Familie Bucket-Strategy) |
| 6.2 Reclamation-Policy | `default_lookup/prt_art_62_reclamation_default.hpp` | **(default-lookup)** | cache-engine/reclamation/rcu_reclaim/ + concepts/mechanics/comdare_rcu_mechanic.hpp |
| 6.3 NUMA-Affinity | `default_lookup/prt_art_63_numa_default.hpp` | **(default-lookup)** | cache-engine/concepts/numa_affinity.hpp (V32.EE.5 NEU) |
| 6.4 Huge-Page-Policy | `default_lookup/prt_art_64_huge_page_default.hpp` | **(default-lookup)** | cache-engine/allocators/portable_aligned_alloc.hpp |
| 6.5 Free-List-Strategy | `allocator/pool_router.hpp` (Bucket-Routing) | **(b)** Neu-Impl | (eigene Bucket-Strategy Frage 15 GPT) |
| 7 PREFETCH | `prefetch/distance_estimator.hpp` + `path_oriented_prefetch.hpp` + `redirect_prefetch.hpp` + V31.K6 `legacy_reimpl/P27/hierarchical_bundle_prefetch.hpp` | **(b)** Neu-Impl + **(a)** P27-Reuse | (Distance-Estimator eigen + HierarchicalBundlePrefetcher Reuse von P27 Zhang) |
| 8.1 Concurrency-Pattern | `concurrency/olc_with_reserved_blocks.hpp` | **(b)** Neu-Impl Kombination | (OLC + Reserved-Value-Blocks-Mechanik) |
| 8.2 Locking-Mode | `default_lookup/prt_art_82_locking_default.hpp` | **(default-lookup)** | cache-engine/concepts/locking_mode.hpp (V32.EE.5 NEU) |
| 9 ISA | `default_lookup/prt_art_9_isa_default.hpp` | **(default-lookup)** | cache-engine IPlatformProbe-Output |
| 10 MEASUREMENT | `measurement/density_tracker.hpp` + `hypothesis_metrics.hpp` | **(b)** Neu-Impl | (Density-Tracker + H1/H2/H3-Metriken) |
| 11 TELEMETRY-COLLECTION | `default_lookup/prt_art_11_telemetry_default.hpp` | **(default-lookup)** | cache-engine/concepts/telemetry/ (Kuehn 11.X1-X4 schon in CE!) |
| 12 HARDWARE-STRATEGY | `default_lookup/prt_art_12_hardware_default.hpp` | **(default-lookup)** | cache-engine/concepts/hardware_strategy.hpp (V32.EE.5 NEU) |
| 13 SCHEDULING-STRATEGY | `default_lookup/prt_art_13_scheduling_default.hpp` | **(default-lookup)** | cache-engine/concepts/scheduling_strategy.hpp (V32.EE.5 NEU) |

## Bilanz pro Status

| Status | Anzahl Achsen |
|---|---|
| (b) Neu-Impl | **10** Achsen (1, 2, 3.A, 3.M, 5, 6.1, 6.5, 7, 8.1, 10) |
| (a) Reuse | **1** Achse (4) + 1 Co-Use mit (b) auf Achse 7 |
| (default-lookup) | **9** Achsen (3.B, 6.2, 6.3, 6.4, 8.2, 9, 11, 12, 13) |
| **TOTAL** | **20 Achsen + Sub-Achsen** |

**Neuartigkeits-Qualifikation:** PRT-ART hat **10 Neu-Implementationen** ueber 10 Achsen.
User-Direktive minimum: "mindestens 1 voellig neuartige Implementation in mindestens 1 Achse".
PRT-ART qualifiziert MEHRFACH als neuartiger Algorithmus.

## Default-Lookup-Mechanik (CEB Auto-Permutator)

Pro Achse mit Status (default-lookup):

1. CEB liest PrtArt-Profil (algorithm_profiles/prt_art.xml)
2. Detect: Achse N nicht explizit definiert (default_lookup_active = true)
3. CEB-AutoPermutator: lookup im CE-Bibliothek-Pfad
4. Filter per IPlatformProbe (nur Host-faehige Variants)
5. Filter per messreihen.xml allowed_variants (User-Limit)
6. Generiere Permutationen pro verbleibender Variant
7. Pro Permutation: ExecuteEngineCommand laufen
8. CompareEngineCommand zum Bestimmen des besten Variants
9. Result-Tabelle: prt-art.<axis> = best-of(variants)

## Querverweise

- O-Phase Spec: `../../../docs/adapters/O_PHASE_PRT_ART_AXES_MIRROR.md` §0 (in Diplomarbeit-Repo)
- AA.3 Korrektur: `../../../docs/uml_planning/Z5_master_index_und_gap_analyse.md` §0
- M-Modell: `../../../docs/architektur/10_schichten_modell_M.md` §0
- BB.1+BB.2 Doxygen-Konvention: `../../../docs/uml_planning/BB_doxygen_mapping_konvention.md`
- V32 Code-Refactoring-Plan: `../../../docs/adapters/V32_CODE_REFACTORING_PLAN.md`

---

**Ende docs/architektur/PRT_ART_AXES_REUSE_MATRIX.md (V32.FF.3 DONE).**
