# comdare-prt-art

> ⚠️ **SUPERSEDED-Hinweis (2026-05-31):** Teile dieser Doku (sowie `PROJECT_LAYER_MAP.md`,
> `STRUCTURAL_CORRECTION_prt_art.md`, `FINDINGS_REV7_6_prt_art.md`) beschreiben das überholte
> **REV-7.6-Submodul-/Hybrid-Engine-Modell**. IST-Stand (code-verifiziert): prt-art ist ein reines
> **Plugin** (`comdare_pruefling.cmake` + `COMDARE_CE_PRUEFLINGE` + `IPrueflingFactory`), KEIN nested
> cache-engine-Submodul; der Mess-Beitrag kommt aus dem compile-time Slot-Merge → DLL-Codegen.
> **IST-treue Single-Source-of-Truth:** `comdare-cache-engine/docs/sessions/architektur-ziele-offene-punkte-ledger.md`
> + `…/20260531-e2e-abnahme-audit-und-entscheidungen.md`.

**PRT-ART** — Probst Redirect Tree / ART hybrid: Hybrid-Suchalgorithmus als experimenteller Pruefling
gegen den Stand der Technik (`comdare-cache-engine`).

## Architektur-Inversion (User-Direktive 2026-05-12)

PRT_ART ist der **Pruefling**, CacheEngine ist das **Werkzeug**:

- `comdare-prt-art/` (dieses Repo) enthaelt die experimentellen PRT-ART-Spezifika
- `comdare-cache-engine/` (paralleles Repo) liefert den Stand der Technik (33 Paper,
  6 Pflicht-Seitentypen, 12 Sub-Engines, Plattform-Auto-Discovery, Decision-Lambda-Trees,
  Concurrency-Disziplinen + Mechaniken, Telemetry-Strategien, Hybrid-Command-Pattern,
  Measurement-Buffer + Measure-Matrix)

## Workflow

1. PRT-ART implementiert experimentelle Methoden parallel zur CacheEngine-Namespace-Struktur
2. Experimente: PRT-ART nutzt CacheEngine zur Verifikation gegen den Stand der Technik
   (`CacheEngineBuilder` permutiert PRT-ART-Bausteine durch alle gueltigen Konfigurationen)
3. Validierte PRT-ART-Methoden wandern in die CacheEngine (werden Teil des Standes der Technik)
4. Final: PRT-ART = bestimmte Permutations-Konfiguration aus CacheEngine-Bausteinen,
   vom CacheEngineBuilder als PRT-ART-Binary kompiliert

## Verzeichnisstruktur (parallele Spiegelung der CacheEngine)

```
prt_art/
├── include/prt_art/
│   ├── nodes/                  PRT-ART eigene Node-Typen (REV 6 §5.17)
│   │   ├── redirect_node.hpp   (P0, CoCo-Trie-inspiriert)
│   │   └── bplus_node.hpp      (P0, eigener kompakter B+)
│   ├── internal_search/        4 Density-abhaengige Such-Strukturen
│   │   ├── array_256.hpp       (Density bis 25%)
│   │   ├── array_65535.hpp     (Density 25-50%)
│   │   ├── vector_u8_u8.hpp    (Density 50-75%)
│   │   └── vector_u16_u16.hpp  (Density >75%)
│   ├── page_structures/        PRT-ART-Konkretisierungen (Custom-Aligned, etc.)
│   ├── interpreters/           PRT-ART-spezifische Interpreter
│   ├── memory_layout/          Virtuelle Memory-Offset-Adressierung (TLB-inspiriert)
│   ├── concurrency/            OLC + reservierte Value-Bloecke
│   ├── value_handle/           PRT-ART-eigene VH (ChainRef etc.)
│   ├── allocator/              4+2 Allocator pools (A/B/C/D + R + V-static/V-dynamic)
│   ├── prefetch/               PRT-ART-eigene Prefetch-Strategien
│   └── measurement/            PRT-ART-spezifische Mess-Hooks
└── src/                        Implementations (sofern noetig)

tests/                          PRT-ART-Tests (gtest)
external/                       Werkzeug-Pfad zu comdare-cache-engine (NICHT eingecheckt)
cmake/                          Build-Module
docs/sessions/                  Session-Dokumentation
```

## Build-Voraussetzung

`comdare-cache-engine` ist als Git-Submodule unter `external/comdare-cache-engine`
eingebunden (User-Direktive 2026-05-12, Ausnahme zu CLAUDE.md S2683 analog S2686b
"BuildSystem Core in Consumer-Projekten").

```sh
# Klonen mit Submodule
git clone --recursive https://github.com/BenniProbst/comdare-prt-art.git
cd comdare-prt-art

# Falls bereits geklont ohne --recursive:
git submodule update --init --recursive

# Build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

## Lizenz

Apache-2.0 (analog comdare-cache-engine).
