# FINDINGS REV 7.6 — comdare-prt-art (2026-05-13)

**Repo-Perspektive:** prt-art-spezifische Findings + Korrekturen aus der
heutigen Drei-Repo-Architektur-Klaerung. Dieses Dokument enthaelt den
**vollstaendigen inhaltlichen Stand** aus prt-art-Sicht (NICHT nur eine
Zusammenfassung).

**Schwester-Dokumente** (gleicher Sprint, andere Repos):
- Diplomarbeit-Master: `FINDINGS_REV7_6_diplomarbeit.md`
- comdare-cache-engine: `FINDINGS_REV7_6_cache_engine.md`
- Habich-Termin-Zusammenfassung: Diplomarbeit `20260508 Termin 7/HABICH_TERMIN7_ZUSAMMENFASSUNG_2026_05_13.md`

---

## §0 Repo-Rolle in der Drei-Repo-Architektur

prt-art ist der **Test-Algorithmus-Pruefling** (User-Direktive 2026-05-13):
- Hybride PrtArtSearchEngine (REV 7.1)
- 8 Schichten Bausteine (Pool, OLC, MultiLevelLayout, PathPrefetch,
  DensityTracker, HypothesisMetrics, ValueHandle, Serialisierung)
- Wird **dreifach** konsumiert:
  1. Direkt fuer Forschungs-Experimente / manuelle Tests
  2. Indirekt via cache-engine Codegen (`search_algorithm_perm`-Achse)
  3. Indirekt via Diplomarbeit-messung_driver (Loop ueber 3 Messreihen)

NICHT in prt-art: Mess-Mechanik (cache-engine) + Auswertungs-Pipeline
(Diplomarbeit/Code/).

---

## §1 Drei-Repo-Architektur (User-Direktive, hoechste Praezision)

```
Diplomarbeit/Code/  =  WAS + AUSWERTUNG    (Anwender)
       │
       ▼ (Submodule, parallel)
comdare-prt-art      =  Test-Algorithmus  (THIS REPO, Pruefling)
       │
       ▼ (Submodule)
comdare-cache-engine =  WIE gemessen wird  (Werkzeug)
```

### Konkrete Aufteilung (User-Direktive 2026-05-13)

| Aspekt | Repo | Begruendung |
|---|---|---|
| PRT-ART Bausteine + Hybride API | prt-art | Pruefling = Kernfunktion |
| Compile-Time-Fallback auf CacheEngine | prt-art | "Erbt von CacheEngine, faellt bei fehlenden Bausteinen zurueck" |
| Mess-Mechanismus | cache-engine | "Die CacheEngine bestimmt WIE getestet wird" |
| Builder-Executable | cache-engine | Nicht prt-art-relevant |
| 3 Messreihen-Definition | Diplomarbeit | Nicht prt-art-relevant |
| XML-Configs | Diplomarbeit | Nicht prt-art-relevant |
| Binary-Ergebnis-Auswertung | Diplomarbeit | Nicht prt-art-relevant |

---

## §2 FINDING 1 — prt-art bleibt **unveraendert** in REV 7.6

**Schwere:** GERING (keine Aenderungen notwendig).

Die heutige Drei-Repo-Architektur-Korrektur hat **keine inhaltlichen
Aenderungen** an prt-art notwendig gemacht.

### Was wurde aktualisiert?

- `PROJECT_LAYER_MAP.md` (commit `01989b9`) wurde um die neue Drei-Repo-
  Architektur erweitert (Section 7: "REV 7.6 — Diplomarbeit-Code-Schicht
  konsumiert prt-art als Submodule").

### Was war bereits aus REV 7.1 vorhanden?

- **Hybride PrtArtSearchEngine** mit drei Spezialisierungen:
  - 1 Param → std::vector-API
  - 2+ Params → std::map-API (mit tuple-Magic fuer N>2)
  - Schreib-Ops → status_t (errno-style, 0=ok)
- `prt_art/identity/status.hpp` mit 11 errno-style Konstanten
- 51 Tests in `tests/unit/test_prt_art_identity.cpp` (MapApi 20 / TupleApi
  4 / VectorApi 19 / Status 1 / Identity 7)
- Submodule auf `comdare-cache-engine` unter `external/`

---

## §3 FINDING 2 — Drei NEUE Konsum-Wege fuer prt-art (REV 7.6)

**Schwere:** ARCHITEKTUR (Stand klargestellt).

### §3.1 Direkt (Forschungs-Experimente, manuelle Tests)
```cpp
#include <prt_art/identity/prt_art_search_engine.hpp>
namespace id = comdare::prt_art::identity;

id::PrtArtSearchEngine<int, std::string> e;
auto rc = e.insert(42, "answer");  // returns int (status_t)
if (rc == id::status_ok) {
    auto v = e.find(42);  // returns std::optional<std::string>
}
```

### §3.2 Indirekt via cache-engine Codegen
Die cache-engine `xml_config_parser` enthaelt `prt_art_v1` als
`search_algorithm_perm`-Achse. Beim Codegen wird ein
`comdare_perm_<fp>.dll`-Modul gebaut, das die hybride API verwendet.

### §3.3 Indirekt via Diplomarbeit-messung_driver
```
Diplomarbeit/Code/messung_driver/main.cpp
    → loopt ueber 3 Messreihen (A/B/C)
    → ruft cache-engine ExperimentDriver auf
        → Codegen erstellt comdare_perm_<fp>.dll inkl. prt-art-Permutationen
            → ModuleLoader laedt + run_workload
                → Diplomarbeit verarbeitet binary Ergebnisse weiter
```

---

## §4 FINDING 3 — Hybride PrtArtSearchEngine (REV 7.1) zentral fuer §10.4

**Schwere:** ZENTRAL (Architektur-Anforderung erfuellt).

Aus der User-Original-Nachricht §10.4 (Master):

> *"Meine pseudocode Definition bedeutet, dass es Varianten meines
> Suchalgorithmus gibt, die sich einerseits an der Baustein-Permutation
> der Cache-Engine in Rekombination der ExecutionEngine/SearchEngine
> bewegen und die sich andererseits automatisch je nach Anzahl der
> Parameter der Suchalgorithmen in ihrer Funktion wandeln. Wenn nur ein
> Parameter angegeben wird, dann ist der typ die value und wir fuellen
> den key typ implizit mit einem hochzaehlenden 64bit unsigned long.
> Werden mehr typ Parameter angegeben, dann ist der erste typ der key,
> alle folgenden Typen formal ein Tupel."*

prt-art-Implementation (REV 7.1):

```cpp
namespace comdare::prt_art::identity {

// 1 Param: std::vector-API
template <typename V>
class PrtArtSearchEngine<V> { ... };

// 2 Params: std::map-API
template <typename K, typename V>
class PrtArtSearchEngine<K, V> { ... };

// N>2 Params: std::map<K, std::tuple<V1, V2, ...>>
template <typename K, typename V1, typename V2, typename... Rest>
class PrtArtSearchEngine<K, V1, V2, Rest...> { ... };

}
```

Schreiboperatoren returnen IMMER `status_t` (errno-style int):
- `status_ok = 0`
- `status_invalid_argument`, `status_duplicate_key`,
  `status_key_not_found`, `status_capacity_exceeded`,
  `status_io_error`, `status_internal_error`, ...

---

## §5 FINDING 4 — Submodule-Layout parallel (Q2)

**Schwere:** ARCHITEKTUR (Layout-Entscheidung).

```
Diplomarbeit/Code/external/
├── comdare-prt-art/         (Submodule, HEAD 3e8044b oder 01989b9)
└── comdare-cache-engine/    (Submodule, HEAD e2dc290 oder d1e79f0)
```

**Vorteil:** Kein Submodule-Tiefe-2-Nesting. Direkte Pins beider Repos.
**Nachteil:** prt-art hat IN seinem eigenen Repo ein
`external/comdare-cache-engine/` Submodule — das wird nicht von
Diplomarbeit konsumiert (Diplomarbeit nutzt ihre OWN cache-engine-
Submodule-Kopie).

**Konsequenz fuer prt-art:**
- prt-art-Standalone-Build benutzt sein eigenes
  `external/comdare-cache-engine/` (aa70a1c).
- Diplomarbeit-Build benutzt eigene cache-engine-Kopie (e2dc290 oder
  neuer).
- Es gibt damit **zwei** cache-engine-Pins, die im Auge behalten werden
  muessen.

---

## §6 FINDING 5 — TikZ-Diagramme (Q3)

**Schwere:** GERING (nicht prt-art-relevant).

User-Entscheidung TikZ-basierte Diagramm-Generierung ist eine
Diplomarbeits-spezifische Aufgabe. prt-art ist nicht betroffen.

---

## §7 FINDING 6 — Hybride PrtArtSearchEngine (REV 7.1) bleibt unveraendert

Die heutige Architektur-Korrektur betrifft NICHT die hybride API.
Verbleibt in `comdare-prt-art/prt_art/include/prt_art/identity/`.
Die prt-art-Bibliothek wird von der Diplomarbeit-Code-Schicht via
Submodule-Pfad konsumiert.

---

## §8 FINDING 7 — ExperimentDriver-Library + main.cpp Wrapper (Q4, cache-engine-seitig)

**Schwere:** GERING (Library in cache-engine, nicht prt-art).

Aus prt-art-Sicht: die heutige cache-engine-Refactoring betrifft prt-art
nicht direkt. Die hybride PrtArtSearchEngine wird ueber die
ExperimentDriver-Library als Codegen-Permutation gebaut, aber prt-art
hat keine eigene Abhaengigkeit auf die Library.

---

## §9 KONSOLIDIERTER STAND DER 3 REPOS

| Repo | Aktueller HEAD | Aenderungen heute (REV 7.6) |
|---|---|---|
| comdare-cache-engine main | `d1e79f0` | REV 7.6 ExperimentDriver-Lib + gitignore-Fix + Diagnose-Restore |
| comdare-prt-art development | `01989b9` | **NUR LAYER_MAP-UPDATE**, keine Code-Aenderungen |
| probst-Diplomarbeit-cache-engine main | `21aa4d8` | REV 7.6 Code/-Skelett + Submodules + Delta 30 + STRUCTURAL_CORRECTION |

---

## §10 Original-Nachricht des Users (verbatim, fuer prt-art relevante Abschnitte)

### §10.1 Variadic-Parameter-Wandlung (zentral fuer prt-art)

> *"Wenn nur ein Parameter angegeben wird, dann ist dieser typ die value
> und wir fuellen den key typ implizit mit einem hochzaehlenden 64bit
> unsigned long. Werden mehr typ Parameter angegeben, dann ist der erste
> typ der key, alle folgenden Typen formal ein Tupel mit allen
> zusammengesetzten values. Im Falle, dass der Key (nur bei 2 oder mehr
> typ Parametern) ein komplexes Objekt und kein Einfachtyp ist, muss
> dieser implizit mindestens mit einer comdare fingerprint Bibliothek-
> Variante und einer ueberladenden Funktion das komplexe Objekt hashen,
> was zu einem binary string statischer Laenge fuehrt."*

**Status fuer prt-art:** DONE in REV 7.1.

### §10.2 PRT-ART als paralleler Stack zur CacheEngine

> *"Strategisch sollten wir die Struktur der CacheEngine in der
> Bearbeitung der stubs und Struktur bevorzugen, weil wir den PRT_ART
> spaeter dort hinein mergen wollen. Die cache engine verfuegt ueber
> einen Stack an Algorithmus-Bausteinen im Bereich Suche und der
> PRT_ART hat dieselbe Struktur mit seinem spezieller zugelassenen
> permutativen parallel-Stack zum CacheEngine Search stack."*

**Status fuer prt-art:** DONE — 8 Bausteine-Schichten als parallele Achsen.

### §10.3 Compile-Time-Fallback

> *"Wenn die Typen aus configuration_permutation_type im Prueflings-
> Algorithmus wie PRT_ART nicht gefunden werden, findet zur compile time
> automatisch ein fallback auf die Bausteine der Cache-Engine Bibliothek
> statt, von der der Algorithmus formal erbt."*

**Status fuer prt-art:** als Konzept dokumentiert, Implementations-Check
in V6 + V7 (siehe Verifikations-Liste).

### §10.4 Test-Daten-Akkumulation-Engine bei SearchEngine-init

> *"Test_data_set_accumulation_engine_type als Klasse, welche die Daten
> geladen hat und die Test-Algo-Interfaces kennt, bereitstellt. In der
> Regel werden alle Testdatensaetze als Implementierung der SearchEngine
> bei Initialisierung dieser eingelesen test_data_set_accumulation_engine_type
> und bereitgestellt."*

**Status fuer prt-art:** OFFEN — heute separate Library (cache-engine),
nicht im PrtArtSearchEngine-Konstruktor verdrahtet.

### §10.5 PRT-ART als spezielle ExecutionEngine

> *"Implizit hat also die execution_engine als typ eine Reihe an
> CacheEngine impliziter Typen, die zur compiletime gesetzt und
> kompiliert werden, und ein search_algorithm ist nichts als eine
> spezielle execution engine (erbt daher compile time statische
> permutationen festgelegter Suchalgorithmus-Bausteine)."*

**Status fuer prt-art:** OFFEN — heute eigene Identity-Klasse, kein
ABI-Inheritance auf `comdare::search_engine`.

---

## §11 Delta-Analyse: Original-Nachricht vs. heutiger prt-art-Stand

### §11.1 KONSISTENT (alle gefordert + heute umgesetzt)

| Anforderung | prt-art Implementations-Stand |
|---|---|
| §10.1 Variadic 1/2/N>2 Params | DONE — REV 7.1 hybride PrtArtSearchEngine |
| §10.1 fingerprint::to_binary_string fuer Keys | DONE |
| §10.2 Permutations-Struktur-Hierarchie parallel | DONE — 8 Schichten |
| §10.2 ABI-Modul-Interface kompatibel | DONE — Codegen liefert prt_art_v1 |
| §10.5 PRT-ART-Bausteine (siehe Bausteine_Matrix.txt) | DONE — 8 Schichten implementiert |

### §11.2 OFFEN aus prt-art-Sicht

| Anforderung | Status | Notwendige Schritte |
|---|---|---|
| §10.3 Compile-Time-Fallback (resolve_baustein) | als Konzept dokumentiert | C++-Implementation der Fallback-Resolution |
| §10.4 TestDataSetAccumulationEngine bei SearchEngine-init | aktuell separate Library | Verdrahtung in `PrtArtSearchEngine`-Konstruktor |
| §10.5 PrtArtSearchEngine erbt von cache-engine `search_engine` ABI | OFFEN — heute eigene Identity-Klasse | Vererbungs-Kette in `prt_art/identity/prt_art_search_engine.hpp` ergaenzen |

---

## §12 Verifikations-Liste (prt-art Perspektive)

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

## §13 Empfehlung fuer Submodule-Pin-Update

Aktuell zeigt `external/comdare-cache-engine` auf `aa70a1c` (REV 7 Phase
6.2.E-6.6). Heutige cache-engine-Aenderungen (REV 7.6):
- `05b41a8` ExperimentDriver-Library + main.cpp Wrapper
- `e2dc290` gitignore-Fix
- `d1e79f0` Diagnose-Restore

**Bumpen**: Wenn prt-art weiterhin die neuesten cache-engine-Aenderungen
benutzen will, sollte das Submodule auf `d1e79f0` (oder neuer)
geupdated werden. **Aktuell ist das NICHT zwingend noetig**, weil prt-art
selber keine Aenderungen erfordert.

---

## §14 Aktueller Repo-State (prt-art development)

| Commit | Beschreibung |
|---|---|
| `01989b9` | REV 7.6 F5 — LAYER_MAP-Update fuer Drei-Repo-Architektur |
| `3e8044b` | PROJECT_LAYER_MAP.md (initial, REV 7.5) |
| `a7d86c7` | Hybride PrtArtSearchEngine REV 7.1 (Vector-/Map-/Tuple-API) + 51 Tests |
| `0522de3` | Submodule-Bump cache-engine zu aa70a1c (REV 7 Phase 6.2.E-6.6) |
| `002c326` | Signaling-Bits-Serialisierung + Linear Value-Buffer (REV 6 §5.17) |

---

## §15 Querverweis

- `PROJECT_LAYER_MAP.md` (prt-art root, REV 7.6 erweitert)
- `STRUCTURAL_CORRECTION_prt_art.md` (Schwester-Dokument im selben Repo)
- cache-engine-Repo: `FINDINGS_REV7_6_cache_engine.md`
- cache-engine-Repo: `STRUCTURAL_CORRECTION_cache_engine.md`
- Diplomarbeit-Repo:
  - `STRUCTURAL_CORRECTION_diplomarbeit.md` (Master mit Original-User-Nachricht)
  - `FINDINGS_REV7_6_diplomarbeit.md` (Master-Findings)
  - `20260508 Termin 7/Phase5_UML_Detail/30_architektur_delta_REV7_6_drei_repo_layer_2026_05_13.md`
  - `20260508 Termin 7/HABICH_TERMIN7_ZUSAMMENFASSUNG_2026_05_13.md`
