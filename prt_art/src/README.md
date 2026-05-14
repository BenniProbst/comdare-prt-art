# prt_art/src/ — Implementations-Slot fuer PRT-ART-Kern

**Stand:** V25.A (2026-05-14) — Verzeichnis bewusst leer.

## Warum leer?

PRT-ART nutzt ein **header-only Pattern** mit selektiver Source-Trennung:

- `prt_art/include/` (30 Header) — Public API + Implementation
  - `prt_art_identity.hpp` — Haupt-Klasse
  - `olc_with_reserved_blocks.hpp` — OLC-Concurrency
  - `pool_*.hpp` — 4+2 Allokator-Pools
- `prt_art/legacy_reimpl/` (14 Sub-Module P11-P27) — SOTA-Module mit
  vollstaendigem Code (KEINE Stubs!)

## Zukunfts-Verwendung

Sobald PRT-ART-Source-Trennung erforderlich wird (z.B. fuer
`.cpp`-Out-of-Line-Implementationen aus dem Header-API):
- Files dort hin verschieben (nicht in `include/`)
- `prt_art/CMakeLists.txt` add_library() mit src/-Pattern erweitern

## Querverweis

- Memory-Direktive `feedback_prt_art_consumes_cache_engine.md`:
  PRT-ART konsumiert CacheEngine, nicht umgekehrt
- `legacy_reimpl/` traegt 14 SOTA-Module (P11-P27) aus Termin-7-Diskussion
