# prt_art/legacy_reimpl/ — Pruefling-Re-Implementations (REV 7.6 V9.4)

**Verschoben aus:** `comdare-cache-engine/prt_art/legacy_reimpl/` (User-Direktive
2026-05-13/14, V8.2 dokumentiert + V9.4 ausgefuehrt).

---

## Zweck

Pruefling-Re-Implementations der TIER-2/3-Paper (P11-P27) als PRT-ART-
Konkretisierungen. Diese Subordner werden im Beitritt-zu-SOTA-Test gegen
die echten cache-engine SOTA-Adapter verglichen.

---

## Layout

```
prt_art/legacy_reimpl/
├── README.md                        (dieses Dokument)
├── P11-CSS-tree/                    Rao/Ross 1999 — CSS-Tree
├── P12-CSB-tree/                    Rao/Ross 2000 — CSB+ Tree
├── P13-Hankins/                     Hankins/Patel 2003 — Node Size
├── P14-Samuel/                      Samuel/Pedersen/Bonnet 2005 — Processor Conscious CSB+
├── P16-Bender-TreeLayout/           Bender/Demaine/Farach-Colton 2002 — Tree Layout
├── P17-Bender-CacheOblivious/       Bender et al 2005 — Cache-Oblivious B-Trees
├── P18-Saikkonen-MultiLevel/        Saikkonen/Soisalon-Soininen 2008 — Multi-Level
├── P19-Saikkonen-LayoutInvariant/   Saikkonen/Soisalon-Soininen 2016 — Layout-Invariant
├── P21-Chen-PrefetchBPlus/          Chen/Gibbons/Mowry 2001 — Prefetching B+ Trees
├── P22-Chen-Fractal/                Chen et al 2002 — Fractal Prefetching
├── P23-Khan-AdaptivePrefetch/       Khan 2010 — Adaptive Prefetch
├── P24-NaderanTahan/                Naderan-Tahan/Sarbazi-Azad 2016
├── P26-Zhang-FGCS/                  Zhang et al FGCS 2024
└── P27-Zhang-ASPLOS-Hierarchical/   Zhang et al ASPLOS 2025 — Hierarchical
```

Hinweise:
- P15 (Graefe/Larson 2001) und P20 (B-Trees-Are-Back) sind als externe
  Adapter in cache-engine `ext/`. Hier nur PRT-ART-Re-Implementations.
- P25/P28-P33 sind Telemetry/Sync/Habich-Quellen ohne separaten Pruefling.

---

## Build-Status

Build optional via `COMDARE_PRT_ART_BUILD_LEGACY_REIMPL` (Default OFF
im prt-art-Root-CMakeLists). Aktivieren nur fuer explizite Pruefling-
Beitritts-Verifikation:

```bash
cmake -B build -DCOMDARE_PRT_ART_BUILD_LEGACY_REIMPL=ON
```

---

## Querverweis

- cache-engine: `prt_art/legacy_reimpl/README.md` (DEPRECATED-Marker)
- Migrations-Doku: `Diplomarbeit/docs/sessions/20260514-1130-v9-anker-vollimplementation.md` §2.1
