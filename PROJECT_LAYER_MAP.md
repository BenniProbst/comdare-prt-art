# PROJECT_LAYER_MAP — comdare-prt-art (REV 7.6, 2026-05-13)

Strategische Strukturübersicht für **manuelles Code-Review**.
Reihenfolge entspricht der **empfohlenen Lese-Reihenfolge**.

> **REV 7.6 Update:** Drei-Repo-Architektur klarer getrennt
> (User-Direktive 2026-05-13). Prt-art bleibt der **Test-Algorithmus-
> Pruefling** mit hybrider PrtArtSearchEngine. Wird jetzt zusaetzlich als
> **paralleles Submodule** vom Diplomarbeit/Code/-Repo konsumiert
> (nicht mehr nur via cache-engine).

---

## 0. Repo-Rolle (REV 7.6 prazisiert)

`comdare-prt-art` ist der **Test-Algorithmus-Pruefling** im Drei-Repo-System:

```
Diplomarbeit/Code/  =  WAS getestet wird + AUSWERTUNG (Anwender)
       │
       ▼ Submodule (parallel)
comdare-prt-art      =  Test-Algorithmus PRT-ART (this repo, Pruefling)
       │
       ▼ Submodule
comdare-cache-engine =  WIE gemessen wird (Werkzeug-Bibliothek)
```

PRT-ART konsumiert `comdare-cache-engine` als Werkzeug-Bibliothek
(Git-Submodule unter `external/comdare-cache-engine`). REV 7 §6 erlaubt
Compile-time-Fallback auf CacheEngine-Bausteine, wenn PRT-ART-Bausteine
fehlen.

**Konsum-Wege (REV 7.6):**

1. **Direkt** (z. B. fuer Tests, Microbenchmarks):
   ```cpp
   #include <prt_art/identity/prt_art_search_engine.hpp>
   comdare::prt_art::identity::PrtArtSearchEngine<int, std::string> e;
   ```
2. **Indirekt via cache-engine Codegen**: `xml_config_parser` enthaelt
   `prt_art_v1` als `search_algorithm_perm`-Achse. Beim Codegen wird ein
   `comdare_perm_<fp>.dll`-Modul gebaut, das die hybride API verwendet.
3. **Indirekt via Diplomarbeit-messung_driver**: Loop ueber 3 Messreihen
   (A/B/C) — prt-art ist eine von vielen Permutationen.

---

## 1. Top-Layer-Hierarchie (8 Schichten von außen nach innen)

```
┌─────────────────────────────────────────────────────────────────────┐
│ L1 IDENTITÄT                                                         │
│    prt_art/identity/                                                 │
│    └─ PrtArtSearchEngine  (hybride Klasse, status.hpp)               │
│    └─ prt_art_identity    (PermutationFlags-Tag → 169-stelliger Hex) │
├─────────────────────────────────────────────────────────────────────┤
│ L2 MEASUREMENT / TELEMETRY                                           │
│    prt_art/measurement/                                              │
│    └─ density_tracker.hpp  (BPlus-Reklassifizierung)                 │
│    └─ hypothesis_metrics.hpp (H1/H2/H3-Heuristiken)                  │
├─────────────────────────────────────────────────────────────────────┤
│ L3 PREFETCH                                                          │
│    prt_art/prefetch/                                                 │
│    └─ path_oriented_prefetch.hpp (Distance-Estimator + Path-Trace)   │
├─────────────────────────────────────────────────────────────────────┤
│ L4 NODE-TYPEN                                                        │
│    prt_art/nodes/                                                    │
│    └─ bplus_node.hpp (Dense-Byte-Page)                               │
│    └─ redirect_node.hpp (Sparse-Patricia-Page)                       │
├─────────────────────────────────────────────────────────────────────┤
│ L5 INTERNAL-SEARCH (4 Density-Bereiche je Page)                      │
│    prt_art/internal_search/                                          │
│    └─ array_256.hpp (Dense ≥ 75%, branch-free)                       │
│    └─ vector_u16_u16.hpp (25-75%, linear scan)                       │
├─────────────────────────────────────────────────────────────────────┤
│ L6 CONCURRENCY                                                       │
│    prt_art/concurrency/                                              │
│    └─ olc_with_reserved_blocks.hpp (OLC + Cache-line Value-Blocks)   │
├─────────────────────────────────────────────────────────────────────┤
│ L7 MEMORY                                                            │
│    prt_art/allocator/                                                │
│    └─ pool_set.hpp (4+2 Pools: A/B/C/D + R + V-static/V-dynamic)     │
│    prt_art/memory_layout/                                            │
│    └─ multi_level_layout.hpp (TLB-Offset + cache-line-aligned)       │
│    prt_art/value_handle/                                             │
│    └─ value_handle.hpp (Inline/External/ChainRef + Cost-Model)       │
├─────────────────────────────────────────────────────────────────────┤
│ L8 SERIALISIERUNG (P9 LOUDS-Format)                                  │
│    prt_art/serialization/                                            │
│    └─ varlen_encoder.hpp, signaling_stream.hpp                       │
│    prt_art/buffer/                                                   │
│    └─ linear_value_buffer.hpp (Tombstone + Compact)                  │
└─────────────────────────────────────────────────────────────────────┘
```

### Lese-Reihenfolge (außen → innen, dependency-respektierend)

| # | Schicht | Files | Begründung |
|---|---|---|---|
| 1 | L7 Memory  | pool_set, multi_level_layout, value_handle | Fundament — alle anderen Schichten allokieren hier |
| 2 | L6 Concurrency | olc_with_reserved_blocks | OLC ist die Grundlage für jeden Multi-Writer-Flow |
| 3 | L4+L5 Nodes+Search | bplus_node, redirect_node, array_256, vector_u16_u16 | Die zwei Page-Typen + ihre internen Suchstrategien |
| 4 | L3 Prefetch | path_oriented_prefetch | Heuristik-Layer über Nodes |
| 5 | L2 Measurement | density_tracker, hypothesis_metrics | Feedback-Loop für Reklassifizierung |
| 6 | L1 Identität | **prt_art_search_engine.hpp** (Hauptklasse) + status.hpp | Komposition aller Schichten + öffentliche API |
| 7 | L8 Serialisierung | varlen_encoder, signaling_stream, linear_value_buffer | Persistenz-Layer (orthogonal, kann am Ende gelesen werden) |

---

## 2. Modul-Abhängigkeiten (gerichtet)

```
PrtArtSearchEngine
   ├── PoolSet           [L7]
   ├── OLC               [L6]
   ├── MultiLevelLayout  [L7]
   ├── PathOrientedPrefetch [L3]
   ├── DensityTracker    [L2]
   ├── HypothesisMetrics [L2]
   ├── ValueHandle       [L7]
   └── fingerprint::to_binary_string  ← cache_engine SUBMODULE
                                         (external/comdare-cache-engine/
                                          cache_engine/include/cache_engine/
                                          fingerprint/fixed_length_fingerprint.hpp)

BPlusNode
   ├── ValueHandle
   └── DensityTracker (für recommended_kind)

RedirectNode
   └── ValueHandle

Array256  ← keine internen Deps (lowest-level)
VectorU16U16  ← keine internen Deps
```

**Wichtig:** Genau **ein** Dependency-Übergang in cache-engine — das
`fingerprint::to_binary_string` in `prt_art_search_engine.hpp`. Sonst ist
PRT-ART komplett selbständig.

---

## 3. Test-Map (welche Test deckt welche Schicht?)

| Schicht | Test-Datei | Tests |
|---|---|---|
| L7 PoolSet | `tests/unit/test_allocator_pools.cpp` | — |
| L7 ValueHandle | `tests/unit/test_value_handle.cpp` | — |
| L7 MemoryLayout | `tests/unit/test_memory_layout.cpp` | — |
| L6 OLC | `tests/unit/test_olc_with_reserved_blocks.cpp` | — |
| L4 Nodes | `tests/unit/test_prt_art_nodes.cpp` | — |
| L5 InternalSearch | `tests/unit/test_internal_search.cpp` | — |
| L3 Prefetch | `tests/unit/test_prefetch_strategies.cpp` | — |
| L2 Measurement | `tests/unit/test_measurement.cpp` | — |
| L1 Identität | **`tests/unit/test_prt_art_identity.cpp`** | **51** |
| L8 Serialisierung | `tests/unit/test_serialization_and_buffer.cpp` | — |

**ctest gesamt: 181 Tests grün (MSVC Debug C++23)**

---

## 4. Paper-Referenzen (in Code-Kommentaren)

| Paper | Code-Stelle | Konzept |
|---|---|---|
| P09 Jacobson 1989 LOUDS | `prt_art/serialization/` | Succinct level-order encoding |
| P29 McKenney RCU 2001 | nicht direkt, aber `OLC` ist konzeptuell verwandt | Read-side-low-cost |
| REV 6 §5.17 | überall in Kommentaren | Bausteine-Konzept |
| REV 7 §4.2(f) | `prt_art_search_engine.hpp` Header-Kommentar | SearchEngine-Dach-Rolle |
| REV 7.1 (heute) | `status.hpp`, hybride Klasse | errno-style + Vector/Map/Tuple-API |

---

## 5. Heutige Änderungen (zum priorisierten Review)

| Datei | Status | Kritikalität für Review |
|---|---|---|
| `prt_art/include/prt_art/identity/status.hpp` | NEU | NIEDRIG (11 Konstanten) |
| `prt_art/include/prt_art/identity/prt_art_search_engine.hpp` | KOMPLETT NEU | **HOCH** — die Hauptklasse |
| `tests/unit/test_prt_art_identity.cpp` | 51 neue Tests | MITTEL — testet Hauptklasse |
| `.idea/` | NEU | NIEDRIG — IDE-Config |

**Empfohlener Review-Fokus:** `prt_art_search_engine.hpp` Zeilen 60–250.
3 Spezialisierungen + Variadic-Magic für Tuple-Modus.

---

## 6. Bekannte Risiken / TODO für Phase 8+

1. **Kein echtes PRT-ART** — `storage_` ist aktuell `std::map<binary_key, V>`.
   Phase 8 ersetzt durch echte Trie-Struktur mit `BPlusNode`/`RedirectNode`.
2. **density_tracker.record() Heuristik** — synthetic_node_id aus
   binary_key-Pointer-Address. Phase 8 ersetzt durch Node-ID aus echter Trie.
3. **path_prefetch_ ungenutzt** im aktuellen lookup — Hook fehlt im
   storage_.find()-Pfad.
4. **Submodule-Pin** zu cache-engine ist auf `aa70a1c` — wenn cache-engine
   gepusht wurde (heute: `a8f12cf`), kann man bumpen.

---

## 7. REV 7.6 — Diplomarbeit-Code-Schicht konsumiert prt-art parallel

Die heutige Drei-Repo-Architektur-Korrektur fuehrt eine NEUE
Anwender-Schicht ein: `Diplomarbeit/Code/` (separates Repo
`probst-Diplomarbeit-cache-engine`).

```
Diplomarbeit/Code/external/comdare-prt-art       ← Submodule (parallel)
Diplomarbeit/Code/external/comdare-cache-engine  ← Submodule (parallel)
```

**prt-art bleibt funktional unveraendert** — es ist weiterhin der
Pruefling-Algorithmus mit der hybriden PrtArtSearchEngine. Die Submodule-
Beziehung zu cache-engine bleibt (`external/comdare-cache-engine/`).

**Wichtige Cross-References:**

- Diplomarbeit-Repo `30_architektur_delta_REV7_6_drei_repo_layer_2026_05_13.md`
- Diplomarbeit-Repo `FINDINGS_REV7_6_2026_05_13.md`
- cache-engine `e2dc290` (gitignore-Fix) — Bumpen empfohlen
- cache-engine `<heute REV 7.6 Diagnose-Restore>` — auch bumpen

---

## 8. Build-Verifikation

```bash
cd comdare-prt-art
cmake -B build-msvc                   # MSVC Visual Studio 17 2022
cmake --build build-msvc --config Debug
cd build-msvc && ctest -C Debug --output-on-failure
# → 181/181 tests passed
```

CLion: `Open Project` auf Repo-Root → CMake-Profile **Debug** / **MSVC-Debug**
automatisch erkannt. RunConfigurations: 4 vorkonfigurierte (Identity, Nodes,
ValueHandle, all-ctest).
