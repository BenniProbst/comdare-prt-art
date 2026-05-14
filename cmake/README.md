# cmake/ — Reserviert fuer prt-art-spezifische CMake-Module

**Stand:** V25.A (2026-05-14) — Verzeichnis bewusst leer, vorgesehen fuer:

- Find-Module fuer prt-art-spezifische externe Abhaengigkeiten
- CMake-Function-Helpers fuer PRT-ART Adapter-Build
- Plattform-spezifische ISA-Detection-Snippets

Aktuell nutzt prt-art die CMake-Module aus `external/comdare-cache-engine/cmake/`
(via Submodule-Konsumption). Sobald prt-art-eigene CMake-Logik benoetigt wird,
landet sie hier.

## Konvention

Pro Modul eine `.cmake`-Datei mit:
- Header-Kommentar (Zweck, Eingangs- + Ausgangs-Variablen)
- `include_guard(GLOBAL)`-Marker
- Defensives Coding (Fallback-Pfade)

## Querverweis

- cache-engine `cmake/` enthaelt die Stand-der-Technik-Hilfsmodule
- `tools/permutation_codegen/` (cache-engine) ist das Template-Pattern
