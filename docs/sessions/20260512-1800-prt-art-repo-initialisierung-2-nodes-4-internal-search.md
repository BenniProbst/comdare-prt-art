# 2026-05-12 18:00 — comdare-prt-art Repo-Initialisierung + 2 Node-Typen + 4 internal search

**Session-Typ:** Repo-Setup + erste PRT-ART-Spezifika (REV 6 §5.17)
**User-Direktive:** "Die Cache Engine ist ein submodule des PRT_ART und sollte auch so konfiguriert werden"

---

## 1. Architektur-Inversion (User-Hinweis 2026-05-12 Abend)

**FALSCH (vorher):** PRT-ART-Inhalte lagen in `comdare-cache-engine/prt_art/`.
Cache-Engine hatte plotzlich einen PRT-ART als Sub-Modul.

**RICHTIG (jetzt):**
- `comdare-prt-art/` (dieses Repo) ist der **Pruefling**
- `comdare-cache-engine/` (Submodule unter `external/`) ist das **Werkzeug**
- PRT-ART konsumiert CacheEngine
- Validierte PRT-ART-Methoden wandern spaeter in CacheEngine als "Stand der Technik"
- Final: PRT-ART = bestimmte Permutations-Konfiguration aus CacheEngine, kompiliert vom CacheEngineBuilder als Binary

---

## 2. Repo-Setup

```
comdare-prt-art/
├── .git/                                       (NEU: eigenes Repo, branch development)
├── .gitignore
├── .gitmodules                                 (CacheEngine-Pin)
├── CMakeLists.txt                              (cmake_minimum_required 3.28, C++23)
├── README.md
├── prt_art/include/prt_art/
│   ├── nodes/                                  (REV 6 §5.17 — 2 Node-Typen)
│   │   ├── redirect_node.hpp                   (P0, CoCo-Trie-inspiriert)
│   │   └── bplus_node.hpp                      (P0, kompakter B+, density-aware)
│   ├── internal_search/                        (REV 6 §5.17 — 4 Density-Schwellwerte)
│   │   ├── array_256.hpp                       (Density bis 25%)
│   │   ├── array_65535.hpp                     (Density 25-50%)
│   │   ├── vector_u8_u8.hpp                    (Density 50-75%)
│   │   └── vector_u16_u16.hpp                  (Density >75%)
│   ├── page_structures/                        (Spiegel der CacheEngine-Konkretisierungen, leer)
│   ├── interpreters/                           (analog)
│   ├── memory_layout/                          (Virtual-Memory-Offset-Adressierung — TBD)
│   ├── concurrency/                            (OLC + reservierte Value-Bloecke — TBD)
│   ├── value_handle/                           (PRT-ART eigene VH inkl. ChainRef — TBD)
│   ├── allocator/                              (4+2 Allocator pools A/B/C/D + R + V-static/V-dynamic — TBD)
│   ├── prefetch/                               (PRT-ART-eigene Strategien — TBD)
│   └── measurement/                            (PRT-ART-spezifische Mess-Hooks — TBD)
├── tests/unit/
│   ├── CMakeLists.txt                          (FetchContent gtest 1.15.2)
│   ├── test_prt_art_nodes.cpp                  (RedirectNode + BPlusNode)
│   └── test_internal_search.cpp                (Array<256>/<65535>, Vector<u8,u8>/<u16,u16>)
├── external/
│   └── comdare-cache-engine/                   (Git-Submodule auf github.com/BenniProbst/comdare-cache-engine)
├── cmake/                                      (eigene Module — TBD)
└── docs/sessions/
    └── 20260512-1800-prt-art-repo-initialisierung-2-nodes-4-internal-search.md
```

---

## 3. Submodule-Konvention (Ausnahme zu CLAUDE.md S2683)

User-Direktive 2026-05-12 Abend: CacheEngine als `git submodule` in PRT_ART
einrichten — analog der S2686b-Ausnahme "BuildSystem Core als Sub-Repo in
Consumer-Projekten" aus CLAUDE.md.

```sh
git submodule add https://github.com/BenniProbst/comdare-cache-engine.git external/comdare-cache-engine
```

`.gitmodules`:
```
[submodule "external/comdare-cache-engine"]
    path = external/comdare-cache-engine
    url = https://github.com/BenniProbst/comdare-cache-engine.git
```

CMake bindet via Pfad-Variable:
```cmake
set(COMDARE_CACHE_ENGINE_DIR "${CMAKE_SOURCE_DIR}/external/comdare-cache-engine")
target_include_directories(comdare_prt_art_core INTERFACE
    "${COMDARE_CACHE_ENGINE_DIR}/cache_engine/include"
    "${COMDARE_CACHE_ENGINE_DIR}/cache_engine/subsystems"
    "${COMDARE_CACHE_ENGINE_DIR}/prt_art/include")
```

---

## 4. 2 PRT-ART Node-Typen (REV 6 §5.17)

### 4.1 RedirectNode (P0)

CoCo-Trie-inspiriert. Speichert `rest_suffix` und `terminal_handle`.
- `try_match(remaining_key) → matched_byte_count`
- `is_complete_match(remaining_key) → bool`

### 4.2 BPlusNode (P0)

Kompakter B+-aehnlicher Verzweigungs-Knoten. Density-Schwellwerte 25/50/75% steuern
welche `InternalSearchKind` der Knoten verwendet:
- 0-25%: `Array256`
- 25-50%: `Array65535`
- 50-75%: `VectorU8U8`
- >75%: `VectorU16U16`

`recommended_kind()` liefert die anhand aktueller Density empfohlene Kind-Variante.

---

## 5. 4 internal search types (REV 6 §5.17)

| Klasse | Typ | Density | Kapazitaet | Lookup |
|--------|-----|---------|------------|--------|
| `Array256` | direkt-adressiert | bis 25% | 256 | O(1) |
| `Array65535` | direkt-adressiert | 25-50% | 65535 | O(1) |
| `VectorU8U8` | sortiertes Vector | 50-75% | 256 | O(log n) Binary Search |
| `VectorU16U16` | sortiertes Vector | >75% | 65535 | O(log n) Binary Search |

Density-Schwellwerte bilden Partition (25 = max von Array256 = min von Array65535,
analog 50 und 75) — verifiziert in `test_internal_search.cpp::DensityThresholdsFormPartition`.

---

## 6. Drei-Repo-Pflicht-Disziplin

User-Direktive: "commit und push bei Diplomarbeit git, cache engine git, prt_art git
jeweils REGELMAESSIG PFLICHT".

| Repo | Status nach Session |
|------|---------------------|
| `comdare-cache-engine` | Commit 5c21058 (Phase 6: Termin-7 Migration INK-1..INK-8 + Sub-Engines + 6 Pflicht-Seitentypen + F-EXTRA-5 Fix) — gepusht zu github.com/BenniProbst/comdare-cache-engine main |
| `comdare-prt-art`      | Initial commit (Repo-Setup + Submodule-Pin + 2 Node-Typen + 4 internal search) — branch development |
| Diplomarbeit           | TBD im naechsten Schritt (neue REV6-Doku + jpgs aus Termin 7) |

---

## 7. Was nicht in dieser Session

- `value_handle/`, `memory_layout/`, `concurrency/`, `allocator/`, `prefetch/`,
  `measurement/` Sub-Module sind angelegt aber leer
- 6 Pflicht-Seitentypen aus REV 5 K05 sind aktuell nur in `comdare-cache-engine/prt_art/`
  vorhanden (sollten spaeter PRT-ART-spezifische Konkretisierungen hier ergaenzt werden)
- ABI-stabiles C++23-Modul-Interface (Phase 7 / CacheEngineBuilder)

---

## 8. Naechste Inkremente

1. ValueHandle (Inline / External / ChainRef) als PRT-ART-eigene Konkretisierung
2. Memory-Layout: TLB-inspirierte virtuelle Offset-Adressierung
   `position = sum(k[i] * 256^(k_count-1-i))`
3. 4+2 Allocator-Pools (A/B/C/D + R + V-static/V-dynamic)
4. OLC + reservierte Value-Bloecke
5. PRT-ART als komponierte ISearchEngine, die CacheEngine-Bausteine nutzt
6. Experiment-Loop: PRT-ART vs Stand der Technik
