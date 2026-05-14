# prt_art/codegen/templates/ — Pruefling-Body-Templates (REV 7.6 V18.2)

**Anlass:** V17.1 Codegen-Template-Substitution + V18.1 Multi-Path-Lookup.

Dieses Verzeichnis enthaelt Body-Templates fuer **Pruefling-Algorithmen**
in prt-art. Sie werden vom cache-engine `CodegenEngine` gefunden, wenn
`ExperimentDriverOptions::prt_art_root` auf das prt-art-Repo zeigt.

---

## Pattern (analog cache-engine SOTA-Templates)

Pro Profile.id ein File `<id>_body.hpp.template` mit:
- `namespace comdare::cache_engine::builder::generated`
- `struct ProfileModuleBody` mit Konstruktor + run_workload + pull_live_counters
- ABI-konform via `comdare_workload_descriptor_v1` /
  `comdare_measurement_record_v1` / `comdare_hw_counters_v1`

---

## Vorhandene Templates

| Template | Profile.id | Pruefling-Implementation |
|---|---|---|
| `prtart_body.hpp.template` | prtart | hybride `PrtArtSearchEngine<...>` |

---

## Folge-Phase

V19+ kann hier weitere Pruefling-Algorithmen hinzufuegen, ohne
cache-engine zu modifizieren. Multi-Pruefling-Vergleich funktioniert
automatisch via algorithm_profiles in beiden Repos.

---

## Querverweis
- cache-engine V17.1: `cache_engine/builder/codegen/codegen.cpp` Multi-Path-Lookup
- cache-engine V17.2: `cache_engine/builder/codegen/templates/` (8 SOTA)
- prt-art V8.10: `prt_art/algorithm_profiles/prtart_pruefling.profile.xml`
