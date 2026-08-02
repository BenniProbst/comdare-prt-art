# Architektur-Ziele- / Offene-Punkte-Ledger - comdare-prt-art (repo-lokale Sicht)

> **VORRANG: das super-LEDGER (`docs/DIPLOMARBEIT-ZIELE-OFFENE-PUNKTE-LEDGER.md`) gewinnt bei Widerspruch;
> dieses Ledger ist die repo-lokale Sicht.**

**Angelegt:** 2026-08-02 | **Paket:** A16-P3 (E-02 Vier-Ledger-Struktur) | **Basis-Commit:** `bd7111d`
**Freigabe:** Owner-Rueckfrage 7 vom 02.08. ("E-24, E-19, E-02, E-23, E-21 sind jetzt PFLICHT") +
Owner-GO ~17:4x ("volles Go fuer ALLE geplante offene Arbeit wie vorgegeben").
**AUDIT-PFLICHT:** Jeder Audit-/Inventur-/Goal-Lauf liest ALLE VIER Ledger (super docs/DIPLOMARBEIT-ZIELE-OFFENE-PUNKTE-LEDGER.md + ce docs/ledger-sections/*.md inkl. goal-v6-luecken-ledger.md + prt-art docs/ledger-sections/*.md + thesis docs/ledger-sections/*.md).
**Vier-Ledger-Doktrin:** super | comdare-cache-engine | comdare-prt-art (diese Datei) | thesis/diplomarbeit.
Das Cluster-Ledger (`Cluster/docs/sessions/20260531-00-37-003158-architektur-ziele-offene-punkte-ledger.md`)
ist das **5.** Ledger und liegt in **Infra-Hoheit** - es steht ausserhalb der Vier-Ledger-Doktrin und darf von
keinem Audit als deren Teil eingefordert werden.

**Schreibweise:** ASCII-only (Projekt-Kanon). Owner-Zitate sind hier ASCII-transliteriert; der verbindliche
Original-Wortlaut steht im super-Session-Doc
`docs/sessions/20260802-OWNER-entscheide-hybrid-tier-stempel-regression-os-unterachsen.md`.

---

## 0. Nutzungs-Disziplin

1. Dieses Ledger traegt **nur repo-lokale** Ziele/offene Punkte von `comdare-prt-art`. Projekt-Steuerung,
   Gates, Trigger-Sequenz und Lager-Strecke leben ausschliesslich im super-LEDGER.
2. Kein Punkt gilt als erledigt ohne **literalen** Beleg (Datei:Zeile, Commit, Test-Ausgabe).
3. Bei Widerspruch zum super-LEDGER: super gewinnt; der Widerspruch wird hier als offener Punkt vermerkt,
   nicht einseitig aufgeloest.
4. Owner-Entscheide werden woertlich im super-Session-Doc persistiert und hier nur **referenziert**.

---

## 1. Rolle des Repos (bindend)

- **PRT-ART ist der PRUEFLING**, die cache-engine ist Werkzeug/SOTA - nie umgekehrt. Vergleich erwuenscht,
  Surrogat verboten. (super-LEDGER:189 "PRT_ART=Pruefling, cache-engine=Werkzeug/SOTA, nie umgekehrt")
- **Separater Pruefling:** prt-art wird **nicht** in den CacheEngineBuilder gemergt; in die Kette geht
  ausschliesslich seine **abstrakte Registry** (super-LEDGER:188 "(c) prt-art = SEPARATER Pruefling").
- **Taxonomie:** `comdare-prt-art` = Modul der ce-Familie **plus** Pruefling-Submodul (super-LEDGER:162).
- **Nie hart verdrahtet:** die Einsetzung erfolgt compile-time ueber `PrueflingSlot` in genau EINE Achse
  (super-LEDGER:189; ce-Seite: `libs/cache_engine/anatomy/pruefling_merge.hpp` - `PrueflingSlotConcept`).

### 1.1 Ist-Stand der Registry (Beleg)

`prt_art/algorithm_profiles/prt_art_axis_registry.xml` (`engine="prt_art" schema="1"`, generiert von
`prt_art/registry_gen`, **nicht** von Hand editieren) traegt heute **5 Achsen**:

| Achse | Slot | category | genus | golden_wired |
|---|---|---|---|---|
| page_type | axis_01 | pruefling_slot | SearchAlgorithm | false |
| prefetch | axis_07 | pruefling_slot | SearchAlgorithm | false |
| telemetry | axis_11 | pruefling_slot | SearchAlgorithm | false |
| value_handle | axis_14 | pruefling_slot | SearchAlgorithm | false |
| path_compression | axis_02 | golden_merge_slot | SearchAlgorithm | **true** (ce-Baustein `PatriciaPathCompression`) |

Konsequenz: **genau ein** Organ ist golden verdrahtet (die SOTA-Merge-Serie ueber `path_compression`);
die vier repo-eigenen Slots sind Slot-Demonstration.

### 1.2 Einhaengung in die Kette (Beleg)

- ce-seitig: `comdare-cache-engine/CMakeLists.txt:124` - `COMDARE_CE_PRUEFLINGE`
  ("Semikolon-Liste der zu ladenden Pruefling-Repo-Verzeichnisse"); jedes Pruefling-Repo stellt
  `comdare_pruefling.cmake` in seiner Wurzel bereit (hier vorhanden).
- Fixture-Kopie + Sync-Gate: `comdare-cache-engine/tests/unit/thesis_tiere/prt_art_axis_registry.xml` ist
  eine **Kopie** dieses Repos (Layering: ce darf prt-art nicht konsumieren); Byte-Gate =
  super-ctest `test_fixture_sync_prt_registry` (`Code/tests/CMakeLists.txt:151`,
  `Code/tests/fixture_sync_check.cmake`). **Jede Registry-Regeneration hier erzwingt den Sync dort.**
- CI: `.gitlab-ci.yml` -- Ist (nachgezaehlt 02.08., der REV-10-Kopfkommentar der Datei ist STALE): 3 Lint-Jobs (lint:secrets/format/static) + 2 unbedingte Build-Jobs (build:standalone:58, build:registry-roundtrip:79) + sanitize:asan-ubsan:104 (unbedingt) + build:clang:92 (opt-in COMDARE_CLANG_MATRIX); alles bare-metal
  (`comdare_prt_art_core` ist INTERFACE-Library, wird nur ueber die Test-Consumer kompiliert).

---

## 2. Bindende Doktrinen aus den Owner-Entscheiden vom 02.08.

### D1 - Merge laeuft GEGEN die Organ-Achsen (Owner-Entscheid 2 / Q2)

Der PRT-ART-Pruefling laeuft **merge gegen die Organ-Achsen**. Daraus folgt fuer den Anatomie-Stempel der
cache-engine: eine **eigene merge-Zeile darf nicht existieren** (sie wird in der A13-Stempel-Regression
entfernt). Meta-Meta-Achsen gehoeren immer zu Mess-, System- oder Organ-Achsen und werden **dynamisch ans
Ende der bestehenden Realm-Zeile** angehaengt.

Owner-Nachtrag Q2 (~12:1x): Die Merge-Strategie **wird durchgefuehrt**, lebt im Stempel aber **nur** ueber
das `e`-Flag der Versionsbezifferung; wo Differenzierung noetig ist, geschieht sie ueber **erweiterte
hierarchische Algorithmus-Namen** nach dem Muster `prt-art.memory.abc@1.0.0` - die cache-engine nutzt
dasselbe Namens-Muster.

**Repo-lokale Pflicht daraus:** kuenftige prt-art-Algorithmus-Namen folgen diesem hierarchischen Muster;
byte-verschiedene Merge-Konstellationen duerfen **nie** namensgleiche Organ-Segmente tragen (die CT-Wache
dafuer baut A13-M3 ce-seitig). **Der Stempel selbst ist ce-Territorium - hier nur die Namens-Pflicht.**

### D2 - Versions-Flag-Grammatik: alle Versionen enden auf `c` oder `ce` (Owner-Q3-final, ~17:0x)

Owner-Wortlaut (Original s. Session-Doc; hier ASCII-transliteriert):
"Die Kurzform ist verboten, Versionierungen sind einheitlich und immer 3-Stellig und beginnen mit 'v'.
Das 'e' ist eine Flag und kann spaeter gegen andere Flags wie 'g' fuer GPU, 'c' fuer CPU, 'f' fuer FPGA und
'n' fuer NPU code erweitert werden. Wir produzieren nur CPU code, daher muessen alle Versionen mit 'c' oder
'ce' enden."

Bindend fuer **jeden** Achsen-Algorithmus dieses Repos:

1. Form `vX.Y.Z` + Flags - immer dreistellig, immer mit `v` beginnend. **Kurzformen (`v1`, `v1e`) sind
   verboten** (Parser-seitig Sentinel).
2. Flag-Position ist erweiterbar: `c`=CPU | `g`=GPU | `f`=FPGA | `n`=NPU; `e`=experimental als **Zusatz**-Flag.
   Reihenfolge: Hardware-Flag zuerst, dann optional `e`.
3. CPU-only-Pflicht: **alle** Versionen enden auf `c` oder `ce` (z. B. `v1.0.0c`, `v2.3.4ce`). Eine Version
   ohne Hardware-Flag ist ungueltig.
4. Das `e` markiert **experimentelle Achsen-Algorithmen aus einem Pruefling** - also genau die Klasse, die
   dieses Repo beitraegt (Owner-Entscheid 2).

**Ist hier:** die generierte Registry `prt_art/algorithm_profiles/prt_art_axis_registry.xml` traegt heute
**kein** Versions-Attribut (`grep -c version` = 0). Die Flag-Grammatik hat in diesem Repo damit noch
**keinen Traeger** -> siehe offener Punkt **M2b**. Grammatik-Parser und CT-Wachen entstehen ce-seitig
(A13-M1/M1b); dieses Repo zieht mit dem Registry-Generator nach, sobald das Schema steht.

### D3 - Lizenz-Stand: Apache-2.0

- `LICENSE` (Apache License 2.0) ist seit Commit `7f6c703` (02.08.2026, Owner-Entscheid "PRT-ART=Apache")
  im Repo; Copyright-Zeile im Anhang: "Copyright 2026 BEP Venture UG (Marke Comdare)".
- Konsistent zur Lizenz-Matrix der Thesis (Rahmen-Pass 02.08.): **DA-Repo + PRT-ART = Apache-2.0**,
  ce-Kern = duales Modell (Rechte BEP Venture UG, Forschung frei, Business gesondert lizenziert),
  Thesis-Text = privates Copyright.
- Offene Restpunkte dazu: **M1**.

---

## 3. Offene Punkte - MAJOR

> Die 3 Review-Majors M-PA-01..03 des aelteren REVIEW-BERICHTs sind GEFIXT (super-LEDGER:512/513); die offenen Majors unten sind daher NEU erhoben, keine Fortschreibung.

### M1 - LICENSE-Historie schliessen (offen, mittel)

- **Ist:** `LICENSE` existiert erst seit `7f6c703` (02.08.2026); das Repo besteht seit 2026-05-12
  (`docs/sessions/20260512-1800-prt-art-repo-initialisierung-...md`). Fuer den Zeitraum davor gibt es
  **keine** Lizenz-Erklaerung im Repo.
- **Ist:** Datei-Header-Abdeckung unvollstaendig - `28` von `68` `.hpp`/`.cpp` unter `prt_art/` tragen
  `SPDX-License-Identifier` (gezaehlt 02.08.2026 auf `bd7111d`).
- **Soll:** (a) rueckwirkende Geltung/Copyright-Halter im Repo dokumentieren (Owner-Entscheid, nicht
  Agenten-Entscheid); (b) SPDX-Header-Abdeckung auf 100 % oder eine bewusst begruendete Ausnahme-Liste;
  (c) Abgleich mit der Thesis-Lizenz-Matrix als **einmalige** Konsistenz-Pruefung vor Abgabe.
- **Bemerkung:** Lizenz-Fragen sind **Owner-Territorium**; hier nur Buchfuehrung des offenen Zustands.

### M2 - PRT-ART-Mess-Anbindung (offen, hoch)

- **M2a Ergebnis-Weg:** Der Pruefling produziert heute **keinen** eigenen Mess-Rueckschrieb; die
  Mess-Kette (CEB -> CSV -> `measurement/<YYYYMMDD-HHMMSS>/`) ist super-/ce-seitig verankert. Offen ist,
  **auf welchem Weg** Pruefling-Ergebnisse in das Mess-Schema (Owner-KERN `replace`/`merge`/`fulljoin`)
  eingehen und wie sie im Lager identifizierbar bleiben.
- **M2b Versions-Traeger:** Die generierte Registry traegt kein Versions-Attribut -> die `c`/`ce`-Flag-Pflicht
  (D2) ist hier heute **nicht abbildbar**. Nachzug an `prt_art/registry_gen`, sobald das ce-Schema (A13-M1b)
  steht. **Achtung Byte-Ereignis:** jede Registry-Regeneration zieht `test_fixture_sync_prt_registry` nach.
- **M2c Sichtbarkeit im golden-Lauf:** 4 der 5 Achsen sind `golden_wired="false"` - der Pruefling ist damit
  im golden-Bestand nur ueber `path_compression` vertreten. Ob das der Soll-Zustand fuer die Auswertung ist,
  ist **offen** und im super-LEDGER zu entscheiden (Mess-Umfang = Projekt-Steuerung).
- **Abgrenzung:** kein Punkt hiervon wird repo-lokal entschieden; dieses Ledger haelt nur den Zustand.

### M3 - E-24-Gattungs-Schnitt (offen, gated)

- **Kontext:** E-24 (Gattungs-Vollausbau `SearchAlgorithm` + `Container`) ist der **LETZTE ABI-Schritt des
  Projekts** und laeuft im EINEN koordinierten 4->5-ABI-Fenster; vor dem Voll-Bau-Trigger sind nur
  **Verankerung + Design-Dossier** Pflicht (super-LEDGER / Bauplan 02.08.).
- **Repo-Bezug:** alle 5 Registry-Achsen dieses Repos stehen heute auf `genus="SearchAlgorithm"`. Ob und
  welche Pruefling-Slots einer **Container**-Gattung (Set/Sequence/Adapter/View) zugeordnet werden, ist
  **offen** und Teil des E-24-Dossiers.
- **Sperre:** Kein ABI-/Gattungs-Touch in diesem Repo vor dem freigegebenen ABI-Fenster - jeder frueherer
  Eingriff entwertet den Voll-Bau-Bestand.

---

## 4. Schnittstellen (nur referenziert, hier nicht entschieden)

- **A13 Stempel-Regression (ce):** merge-Zeile faellt, Meta-Meta-Anhaengung, `e`/Flag-Versionierung,
  POD-Layout-Wechsel. Der Anatomie-Stempel ist **ce-Territorium**; dieses Repo liefert nur Namen + Versionen.
- **Lager/Skip-Identitaet:** Stempel = Lager-Identitaet; jede Namens- oder Versions-Aenderung hier ist ein
  Byte-Ereignis fuer den Lager-Bestand. Terminierung ausschliesslich ueber das super-LEDGER.
- **Thesis:** Lizenz-Matrix + Pruefling-Framing werden in der Thesis gefuehrt; Aenderungen hier sind dort
  nachzuziehen (Verweis, keine Doppelquelle).

---

## 5. Quellen (Voll-Pfade)

- super-LEDGER: `probst-diplomarbeit-cache-engine/docs/DIPLOMARBEIT-ZIELE-OFFENE-PUNKTE-LEDGER.md`
- ce-Ledger: `.../Code/external/comdare-cache-engine/docs/ledger-sections/architektur-ziele-offene-punkte-ledger.md`
  (+ Zusatz-Ledger `goal-v6-luecken-ledger.md` im selben Verzeichnis)
- prt-art-Ledger: **diese Datei**
- thesis-Ledger: `.../thesis/diplomarbeit/docs/ledger-sections/architektur-ziele-offene-punkte-ledger.md`
- Owner-Entscheide 02.08. (Wortlaut):
  `.../docs/sessions/20260802-OWNER-entscheide-hybrid-tier-stempel-regression-os-unterachsen.md`
- Bauplan 02.08.: `.../docs/sessions/20260802-BAUPLAN-owner-entscheide-synthese-kritischer-pfad.md`
