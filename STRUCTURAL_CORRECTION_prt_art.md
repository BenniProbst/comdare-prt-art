# STRUCTURAL_CORRECTION REV 7.6 — comdare-prt-art (2026-05-13)

**Repo-Perspektive:** Wie die heutige Drei-Repo-Architektur-Korrektur
prt-art betrifft. Dieses Dokument enthaelt den **vollstaendigen
inhaltlichen Stand** aus prt-art-Sicht (NICHT nur eine Zusammenfassung)
— die Master-Version in der Diplomarbeit ist parallel gehalten.

**Schwester-Dokumente** (gleicher Sprint, andere Repos):
- Diplomarbeit-Master: `STRUCTURAL_CORRECTION_diplomarbeit.md`
- comdare-cache-engine: `STRUCTURAL_CORRECTION_cache_engine.md`
- Habich-Termin-Zusammenfassung: Diplomarbeit `20260508 Termin 7/HABICH_TERMIN7_ZUSAMMENFASSUNG_2026_05_13.md`

**Anlass:** User-Klarstellung 2026-05-13: die Code-Verzeichnisstruktur
der **Diplomarbeit** wurde zunaechst falsch verortet. Diese Doku
beschreibt aus prt-art-Sicht, welche Konsequenz die Korrektur hat —
**keine Code-Aenderungen**, nur eine neue Konsum-Beziehung.

---

## §1 Die korrekte Drei-Repo-Architektur

```
┌──────────────────────────────────────────────────────────────────────┐
│  Diplomarbeit/Code/  =  WAS getestet wird + AUSWERTUNG                │
│  ─────────────────────────────────────────────────────────────────── │
│  messung_driver/   binary_to_csv/   csv_to_latex/                     │
│  diagram_generator/   latex_to_pdf/                                   │
│  experiment_config/{A,B,C}.xml                                        │
└──────────────────────────────────────────────────────────────────────┘
                       │
                       │ konsumiert als Git-Submodule (parallel zu cache-engine)
                       ▼
┌──────────────────────────────────────────────────────────────────────┐
│  comdare-prt-art  (THIS REPO — Test-Algorithmus-Pruefling)            │
│  ─────────────────────────────────────────────────────────────────── │
│  Hybride PrtArtSearchEngine (REV 7.1)                                 │
│  8 Bausteine-Schichten:                                               │
│   - Pool (4+2 Allocator-Pools)                                        │
│   - OLC (Optimistic Lock Coupling)                                    │
│   - MultiLevelLayout                                                  │
│   - PathPrefetch                                                      │
│   - DensityTracker                                                    │
│   - HypothesisMetrics (H1/H2/H3)                                      │
│   - ValueHandle (Inline/External/ChainRef)                            │
│   - Serialisierung (Signaling-Bits + Linear Value-Buffer)             │
│  Mess-Interfaces fuer das Diplomarbeits-Experiment                    │
└──────────────────────────────────────────────────────────────────────┘
                       │
                       │ konsumiert als Git-Submodule
                       ▼
┌──────────────────────────────────────────────────────────────────────┐
│  comdare-cache-engine  (Werkzeug-Bibliothek)                          │
└──────────────────────────────────────────────────────────────────────┘
```

**User-Direktive 2026-05-13 (relevanter Auszug fuer prt-art):**
> *"Strategisch sollten wir die Struktur der CacheEngine in der
> Bearbeitung der stubs und Struktur bevorzugen, weil wir den PRT_ART
> spaeter dort hinein mergen wollen. Die cache engine verfuegt ueber
> einen Stack an Algorithmus-Bausteinen im Bereich Suche und der
> PRT_ART hat dieselbe Struktur mit seinem spezieller zugelassenen
> permutativen parallel-Stack zum CacheEngine Search stack. Technisch
> gesehen sind daher die processing_strategy_type fuer die
> Rekombinations-Konfiguration, nicht nur auf die execution_engine/
> search_engine, sondern auch auf den PRT_ART im selben parallelen
> Konfigurationsformat anwendbar. Wir erben also aus der CacheEngine
> die Permutations-Struktur-Hierarchie der Algorithmus-Bausteine, wenn
> die Typen aus configuration_permutation_type im Prueflings-Algorithmus
> wie PRT_ART nicht gefunden werden, findet zur compile time automatisch
> ein fallback auf die Bausteine der Cache-Engine Bibliothek statt, von
> der der Algorithmus formal erbt."*

**Konsequenz:** prt-art ist ein **paralleler Stack** zur cache-engine.
Es erbt die Permutations-Struktur-Hierarchie und faellt zur Compile-time
auf cache-engine-Bausteine zurueck, wenn prt-art-eigene Bausteine fehlen.

---

## §2 Die drei Messreihen aus prt-art-Sicht

Die drei Diplomarbeits-Messreihen konsumieren prt-art unterschiedlich:

### §2.1 Messreihe A — PRT-ART vs. Stand-der-Technik

prt-art ist **direkter Pruefling**. Die hybride PrtArtSearchEngine wird
ueber den cache-engine-Codegen als `comdare_perm_<fp>.dll` gegen 8 SOTA-
Adapter (P01 ART, P02 HOT, P03 Masstree, ...) gemessen.

### §2.2 Messreihe B — Bestehende cache-engine-Permutationen

prt-art ist **NICHT involviert** (reine cache-engine-Permutationen).

### §2.3 Messreihe C — Merge alt/neu

prt-art ist **teilweise involviert**: 4 Merge-Punkte verwenden prt-art-
Bausteine in Kombination mit cache-engine-Bausteinen:
- PRT-ART OLC + tcmalloc + ART
- 4+2-Pool + HOT
- PathPrefetch + B2tree
- MultiLevelLayout + Wormhole

---

## §3 Was in prt-art BLEIBT (keine inhaltliche Aenderung in REV 7.6)

| Komponente | Status |
|---|---|
| `prt_art/identity/PrtArtSearchEngine` (REV 7.1 hybride API) | Unveraendert |
| `prt_art/identity/status.hpp` (11 errno-Status-Codes) | Unveraendert |
| 8 Bausteine-Schichten | Unveraendert |
| 51 Tests in `tests/unit/test_prt_art_identity.cpp` | Unveraendert |
| Submodule `external/comdare-cache-engine` | Pin aktuell `aa70a1c` (vor REV 7.6) |

**Wichtig:** prt-art **VERLIERT NICHTS**. Die REV-7.1-API
(Vector/Map/Tuple) bleibt der Stand der Implementation.

---

## §4 Was sich in prt-art geandert hat (nur Doku)

| Commit | Beschreibung |
|---|---|
| `01989b9` | REV 7.6 F5 — `PROJECT_LAYER_MAP.md` Section 7 ergaenzt (Drei-Repo-Konsum) |

**Keine Code-Aenderungen, keine Test-Aenderungen, kein Submodule-Bump.**

---

## §5 Konsequenzen fuer prt-art

### §5.1 prt-art wird jetzt **doppelt** konsumiert

Vorher (REV 7.1-7.5): prt-art wurde nur indirekt via cache-engine-Codegen
benutzt (eine `search_algorithm_perm`-Achse).

Jetzt (REV 7.6): zusaetzlich wird prt-art als **paralleles Submodule**
in `Diplomarbeit/Code/external/comdare-prt-art/` aufgenommen, damit die
Diplomarbeit direkten Zugriff auf die hybride API hat (z. B. fuer
spezielle Diplomarbeits-Tests).

### §5.2 Submodule-Layout (parallel, Q2-Entscheidung)

```
Diplomarbeit/Code/external/
├── comdare-prt-art         ← Submodule (parallel zu cache-engine)
└── comdare-cache-engine    ← Submodule (parallel zu prt-art)
```

prt-art hat sein eigenes Submodule auf cache-engine
(`external/comdare-cache-engine`) — das **bleibt** unveraendert. Die
Diplomarbeit benutzt jedoch ihre **eigene** cache-engine-Submodule-Kopie
(`Code/external/comdare-cache-engine`), NICHT die nested-Variante im
prt-art-Submodule.

### §5.3 prt-art-eigener Build bleibt eigenstaendig

prt-art ist weiterhin als eigenes Repo voll funktional buildbar:
```bash
cd comdare-prt-art
git submodule update --init --recursive
cmake -B build-msvc
cmake --build build-msvc --config Debug
cd build-msvc && ctest -C Debug
# → 51 / 51 prt-art-eigene Tests gruen + cache-engine-Tests
```

### §5.4 Submodule-Pin-Bump optional

Aktuell zeigt `external/comdare-cache-engine` auf `aa70a1c`. Heutige
cache-engine-Aenderungen (REV 7.6):
- `05b41a8` ExperimentDriver-Library + main.cpp Wrapper
- `e2dc290` gitignore-Fix
- `d1e79f0` Diagnose-Restore

**Bumpen**: Wenn prt-art weiterhin die neuesten cache-engine-Aenderungen
benutzen will, sollte das Submodule auf `d1e79f0` (oder neuer) geupdated
werden. **Aktuell ist das NICHT zwingend noetig**, weil prt-art selber
keine Aenderungen erfordert.

---

## §6 Aktueller Repo-State (prt-art development)

| Commit | Beschreibung |
|---|---|
| `01989b9` | REV 7.6 F5 — LAYER_MAP-Update fuer Drei-Repo-Architektur |
| `3e8044b` | PROJECT_LAYER_MAP.md (initial, REV 7.5) |
| `a7d86c7` | Hybride PrtArtSearchEngine REV 7.1 (Vector-/Map-/Tuple-API) + 51 Tests |
| `0522de3` | Submodule-Bump cache-engine zu aa70a1c (REV 7 Phase 6.2.E-6.6) |
| `002c326` | Signaling-Bits-Serialisierung + Linear Value-Buffer (REV 6 §5.17) |

---

## §7 Naechste Schritte aus prt-art-Perspektive

1. **Optional: Submodule-Pin-Bump** cache-engine → `d1e79f0`
   (heutige REV-7.6-Restore)
2. **Optional: Branch-Strategie** — aktueller Branch ist `development`,
   nicht `main`. Pruefen, ob Merge zu `main` ansteht oder ob
   `development` weiterlaufen soll.
3. **Phase 8+:** echte Permutations-Logic in den 54 generierten Module-DLLs
   (statt heutiger Mock-Bodies). Dies erfordert prt-art-Bausteine in
   `run_workload`-Implementationen.
4. **V6 (cross-repo):** PrtArtSearchEngine erbt heute NICHT von
   `comdare::search_engine` ABI. Anforderung §10.8 verlangt das jedoch.
   Aufwand: Vererbungs-Kette in `prt_art/identity/prt_art_search_engine.hpp`
   ergaenzen.
5. **V7 (cross-repo):** `TestDataSetAccumulationEngine` aus cache-engine
   muss im Konstruktor von PrtArtSearchEngine verdrahtet werden.

---

## §8 Original-Nachricht des Users (verbatim, fuer prt-art relevante Abschnitte)

> *Anlass: Phase 6 INK-1 bis INK-8 Migration REV 6 (2026-05-12 1500),
> Pre-Habich-Sprechstunde von 2026-05-08, mit nachgelegter Praezisierung.*

### §8.1 Custom Allokation als gemeinsame Disziplin (CacheEngine + PRT-ART)

> *"Die Cache Engine ist ebenfalls dafuer da, nach bekannten weitreichenden
> Allokationsmethoden Speicher zu verwalten."*

prt-art hat seine **eigenen 4+2-Pools**, kann aber zusaetzlich
cache-engine-Allokatoren (A01-A23) konsumieren.

### §8.2 ABI-stabiles C++23-Interface

> *"Bezueglich der ABI stabilen C++23 interfaces verhaelt es sich so, dass
> jedes kompilierte Experiment eine bestimmte Execution Engine -> Search
> Engine Rekombination ist"*
>
> ```cpp
> std::variant<
>     comdare::search_engine<
>         search_algorithm_type_collection<key, value>,
>         configuration_permutation_type
>     > : comdare::execution_engine<processing_strategy_type>(
>         test_data_set_accumulation_engine_type
>             data_accumulation_benchmark_routines(data_set)
>     )
> >();
> ```

**Konsequenz fuer prt-art:** PrtArtSearchEngine **muss** sich in diese
Hierarchie einfuegen (V6).

### §8.3 Variadic-Parameter-Wandlung (prt-art zentral)

> *"Wenn nur ein Parameter angegeben wird, dann ist dieser typ die value
> und wir fuellen den key typ implizit mit einem hochzaehlenden 64bit
> unsigned long. Werden mehr typ Parameter angegeben, dann ist der erste
> typ der key, alle folgenden Typen formal ein Tupel mit allen
> zusammengesetzten values."*

**Konsequenz fuer prt-art:** Die hybride PrtArtSearchEngine (REV 7.1)
**implementiert genau das**. Drei Spezialisierungen:
- 1 Param → std::vector-API
- 2 Params → std::map-API
- N>2 Params → std::map mit std::tuple-Values

### §8.4 PRT-ART als paralleler Stack zur CacheEngine

> *"Strategisch sollten wir die Struktur der CacheEngine in der
> Bearbeitung der stubs und Struktur bevorzugen, weil wir den PRT_ART
> spaeter dort hinein mergen wollen. Die cache engine verfuegt ueber
> einen Stack an Algorithmus-Bausteinen im Bereich Suche und der
> PRT_ART hat dieselbe Struktur mit seinem spezieller zugelassenen
> permutativen parallel-Stack zum CacheEngine Search stack."*

**Konsequenz fuer prt-art:** Bausteine-Schichten (Pool, OLC, Layout,
Prefetch, Density, Hypothesis, ValueHandle, Serial) sind **parallele
Achsen** zur cache-engine-Permutationsmatrix.

### §8.5 Compile-Time-Fallback (kritisch fuer prt-art)

> *"Wenn die Typen aus configuration_permutation_type im Prueflings-
> Algorithmus wie PRT_ART nicht gefunden werden, findet zur compile time
> automatisch ein fallback auf die Bausteine der Cache-Engine Bibliothek
> statt, von der der Algorithmus formal erbt."*

**Konsequenz fuer prt-art:** PRT-ART erbt formal von cache-engine. Im
Code: `resolve_baustein<Algo, BausteineTag>` zur Compile-Time.

---

## §9 Delta-Analyse: Original-Nachricht vs. heutiger prt-art-Stand

### §9.1 KONSISTENT (alle gefordert + heute umgesetzt)

| Anforderung (§8) | prt-art Implementations-Stand |
|---|---|
| §8.2 ABI-Modul-Interface kompatibel | DONE — Codegen liefert prt_art_v1 als `search_algorithm_perm` |
| §8.3 Variadic 1/2/N>2 Params | DONE — REV 7.1 hybride PrtArtSearchEngine |
| §8.3 fingerprint fuer komplexe Keys | DONE — fixed_length_fingerprint |
| §8.4 Permutations-Struktur-Hierarchie parallel | DONE — 8 Schichten Bausteine |
| §10.8 (Master) `_archive_code_pre_migration/` | NICHT prt-art-relevant |

### §9.2 OFFEN aus prt-art-Sicht

| Anforderung | Status | Notwendige Schritte |
|---|---|---|
| §8.2 PrtArtSearchEngine erbt von `comdare::search_engine` ABI | **OFFEN** — heute eigene Identity-Klasse, kein ABI-Inheritance | Vererbungs-Kette in `prt_art/identity/prt_art_search_engine.hpp` ergaenzen |
| §8.5 Compile-Time-Fallback auf cache-engine-Bausteine | als Konzept | `resolve_baustein<Algo, BausteineTag>` im prt-art Compile-Time |
| §10.11 (Master) TestDataSetAccumulationEngine bei SearchEngine-init | aktuell separate Lib | Verdrahtung in `PrtArtSearchEngine`-Konstruktor |

---

## §10 Verifikations-Liste (prt-art Perspektive)

| # | Test | Status |
|---|---|---|
| V1 | `comdare-prt-art` baut standalone | DONE |
| V2 | 51 prt-art-eigene Tests gruen (MapApi 20 / TupleApi 4 / VectorApi 19 / Status 1 / Identity 7) | DONE |
| V3 | Hybride API mit 1/2/N>2 Params | DONE (REV 7.1) |
| V4 | status.hpp 11 errno-Konstanten | DONE |
| V5 | Cross-Repo: prt-art als Submodule in Diplomarbeit konsumierbar | DONE (parallel layout) |
| V6 | PrtArtSearchEngine erbt von cache-engine `search_engine` ABI | **OFFEN** |
| V7 | TestDataSetAccumulationEngine bei SearchEngine-init verdrahtet | **OFFEN** |
| V8 | Submodule-Pin-Bump cache-engine → d1e79f0 | **OFFEN** (optional) |

---

## §11 Konsum-Wege fuer prt-art (REV 7.6 Vollbild)

### §11.1 Direkt (Forschungs-Experimente, manuelle Tests)
```cpp
#include <prt_art/identity/prt_art_search_engine.hpp>
namespace id = comdare::prt_art::identity;

id::PrtArtSearchEngine<int, std::string> e;
auto rc = e.insert(42, "answer");  // returns int (status_t)
if (rc == id::status_ok) {
    auto v = e.find(42);  // returns std::optional<std::string>
}
```

### §11.2 Indirekt via cache-engine Codegen

Die cache-engine `xml_config_parser` enthaelt `prt_art_v1` als
`search_algorithm_perm`-Achse. Beim Codegen wird ein
`comdare_perm_<fp>.dll`-Modul gebaut, das die hybride API verwendet.

### §11.3 Indirekt via Diplomarbeit-messung_driver
```
Diplomarbeit/Code/messung_driver/main.cpp
    → loopt ueber 3 Messreihen (A/B/C)
    → ruft cache-engine ExperimentDriver auf
        → Codegen erstellt comdare_perm_<fp>.dll inkl. prt-art-Permutationen
            → ModuleLoader laedt + run_workload
                → Diplomarbeit verarbeitet binary Ergebnisse weiter
```

---

## §12 Querverweis

- `FINDINGS_REV7_6_prt_art.md` (heutige Findings im Detail)
- `PROJECT_LAYER_MAP.md` (8-Schichten-Hierarchie der prt-art, REV 7.6 erweitert)
- cache-engine-Repo: `STRUCTURAL_CORRECTION_cache_engine.md`
- cache-engine-Repo: `FINDINGS_REV7_6_cache_engine.md`
- Diplomarbeit-Repo: `STRUCTURAL_CORRECTION_diplomarbeit.md` (Master mit User-Original-Nachricht §10 verbatim)
- Diplomarbeit-Repo: `FINDINGS_REV7_6_diplomarbeit.md` (Master-Findings)
- Diplomarbeit-Repo: `20260508 Termin 7/HABICH_TERMIN7_ZUSAMMENFASSUNG_2026_05_13.md`
