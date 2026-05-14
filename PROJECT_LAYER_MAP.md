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

---

## 10. V8-V11 Delta (REV 7.6 Vollimplementierung, 2026-05-13/14)

### 10.1 Neue Header (prt_art/include/prt_art/identity/)
- `prt_art_search_engine_adapter.hpp` (V8.9) — Komposition statt Inheritance fuer ABI-Konformitaet
  - V9.1: 3 konkrete Subklassen Map/Vector/Tuple
  - V10.3: notify_*-Methoden Skelett mit TODO
  - V11.1: notify_*-Methoden vollstaendig verdrahtet (density_tracker/path_prefetch/hypothesis_metrics)

### 10.2 Erweiterte Komponenten (V11.1 Komponenten-API)
- `density_tracker.hpp` — `note_observation(double)` + `last_external_sample()`
- `path_oriented_prefetch.hpp` — `note_hot_path_bytes(byte*, size_t)` + `total_hot_path_hints()`
- `hypothesis_metrics.hpp` — `adapt_locality(double)` + `total_locality_adapts()`

### 10.3 Neue Verzeichnisse
- `prt_art/algorithm_profiles/` (V8.10) — Pruefling-Profile + permutation_axes_extension
- `prt_art/legacy_reimpl/` (V9.4) — 14 Pruefling-Re-Implementations P11-P27 (verschoben aus cache-engine)

### 10.4 CI
- `.gitlab-ci.yml` (V11.7) — primaer
- `.github/workflows/ci.yml` (V11.7) — synchron

### 10.5 CMake-Optionen
- `COMDARE_PRT_ART_BUILD_LEGACY_REIMPL` (V9.4, default OFF)

### Querverweis
- comdare-prt-art/docs/sessions/20260514-0900-v8-prt-art-abi-inheritance.md

---

## 11. V12-V14 Delta (REV 7.6 weitere Iterationen, 2026-05-14)

### 11.1 PrtArtSearchEngine API-Vervollstaendigung (V12.1 + V12.2 + V13.5)
- **Vector-API +5 Methoden (V12.1):** operator[], emplace_back, swap, rbegin/rend (+ crbegin/crend), assign(InputIt, InputIt)
- **Map-API +8 Methoden (V12.2):** operator[], emplace, try_emplace, insert_or_assign, swap, max_size, rbegin/rend, key_comp/value_comp
- **Map-API +2 Methoden (V13.5):** merge(other) mit cross-engine std::scoped_lock, extract(key) mit optional-Return

### 11.2 PrtArtSearchEngineAdapter ABI-Vertraege (V12.4 + V11.1)
- 3 konkrete Subklassen Map/Vector/Tuple ueberschreiben contains/count/find/clear (V12.4) + notify_density_threshold/hot_path_detected/workload_change (V11.1 vollstaendig verdrahtet, kein TODO)

### 11.3 Komponenten-API-Erweiterungen (V11.1)
- `density_tracker.hpp` — `note_observation(double)` + `last_external_sample()`
- `path_oriented_prefetch.hpp` — `note_hot_path_bytes(byte*, size_t)` + `total_hot_path_hints()`
- `hypothesis_metrics.hpp` — `adapt_locality(double)` + `total_locality_adapts()`

### 11.4 Bug-Fix (V14.1)
- `storage_t` → `storage_map_t` in V12.2 const_reverse_iterator (typedef-Verweis war falsch, von V14.1-Tests aufgedeckt)

### 11.5 Tests (V14.1)
- `tests/unit/test_prt_art_identity.cpp` erweitert um 15 neue Tests:
  - Vector-API V12.1: 5 Tests (operator[], emplace_back, swap, rbegin/rend, assign)
  - Map-API V12.2: 7 Tests (operator[], emplace, try_emplace, insert_or_assign, swap, max_size, key/value_comp)
  - Map-API V13.5: 3 Tests (merge unique-only, extract returns+removes, extract missing nullopt)
- Test-Lauf: 66/66 passed (17 ms)

### 11.6 CI Test-Discovery-Workaround (V14.3)
- Direkte Test-Binary-Ausfuehrung in beiden CI-Configs

### Querverweis
- comdare-prt-art HEAD: e6b79a3 (V14)

---

## 12. REV 7.7 V19-V27 (2026-05-14): Pipeline + Layout + Konsistenz

### 12.1 V19.1 expected_workload-Tag im prtart-Profile
`prt_art/algorithm_profiles/prtart_pruefling.profile.xml`:
```xml
<expected_workload>YCSB_F</expected_workload>
```
Read-modify-write fuer Density-Tracker-Updates.

### 12.2 V18.2 prtart_body.hpp.template (Codegen)
NEU `prt_art/codegen/templates/prtart_body.hpp.template`:
- 3 atomic counters (ops_executed_, hot_path_hits_, density_observations_)
- Konsumiert von cache-engine codegen Multi-Path-Lookup (V18.1)

### 12.3 V24.A Cleanup + CMakePresets
- 5 obsolete `build-msvc-vXX/`-Verzeichnisse entfernt
- `CMakePresets.json` (V6) mit `binaryDir=${sourceDir}/build/${presetName}`
- 4 Presets (msvc-release, msvc-debug, gcc-release, clang-release)

### 12.4 V25.A Audit-Cleanup
2 leere Dirs (`cmake/`, `prt_art/src/`) bekommen README.md statt Loeschung
(Memory-Direktive "niemals Doku loeschen" — V24).

### 12.5 cache-engine V25-V26 Konsumption (via Submodule)
- 30/30 SOTA-Profile (V25.B) — fuer Algorithmus-Vergleich
- 10 Allokator-Profile (V26.A) — fuer Allokator-Vergleich
- 21 Adapter-Skelette (V25.C + V26.B) — `comdare::adapter::*` ALIAS-Targets

### Querverweis V19-V27
- Diplomarbeit/docs/sessions/20260514-3300-v25-anker.md (V25)
- Diplomarbeit/docs/sessions/20260514-3500-v26-anker.md (V26)
- comdare-prt-art HEAD V25.A: afbdd75
- comdare-cache-engine HEAD V27.A: 11ab988
