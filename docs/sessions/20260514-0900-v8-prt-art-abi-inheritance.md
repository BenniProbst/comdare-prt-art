# V8 prt-art ABI-Inheritance (2026-05-14, 09:00)

**Master:** `Diplomarbeit/docs/sessions/20260514-0900-v8-implementations-anker-rev7-6.md`
**Schwester:** `comdare-cache-engine/docs/sessions/20260514-0900-v8-cache-engine-strukturkorrekturen.md`

**Anlass:** User-Klarstellung 2026-05-13/14: PRT-ART erbt formal von der
CacheEngine; CacheEngine vererbt 8 Schichten an PRT-ART; TestDataSet wird
bei SearchEngine-Konstruktion mit-initialisiert.

---

## §1 prt-art-spezifische Aufgaben (V8.9-V8.11)

### V8.9 — PrtArtSearchEngine erbt von comdare::search_engine ABI

**Aktueller Stand:** `prt_art/include/prt_art/identity/prt_art_search_engine.hpp`
ist eine eigenstaendige Identity-Klasse OHNE ABI-Vererbung. Verstoesst
gegen User-Direktive §10.5 / §10.8 in der Master-Doku.

**Soll-Stand:**
```cpp
#include <cache_engine/abi/search_engine.hpp>
#include <cache_engine/abi/configuration_permutation.hpp>

namespace comdare::prt_art::identity {

template <typename... Ts>
class PrtArtSearchEngine
    : public comdare::search_engine<
          comdare::search_algorithm_type_collection<Ts...>,
          PrtArtConfigurationPermutation>
{
public:
    using base_t = comdare::search_engine<
        comdare::search_algorithm_type_collection<Ts...>,
        PrtArtConfigurationPermutation>;

    using key_t   = typename base_t::collection_t::key_t;
    using value_t = typename base_t::collection_t::value_t;

    // Hybride API (REV 7.1, beibehalten)
    int  insert(key_t const& k, value_t const& v);
    auto find(key_t const& k) const noexcept -> std::optional<value_t>;
    int  erase(key_t const& k);
    auto size() const noexcept -> std::size_t;
};

}  // namespace
```

User-Direktive: *"Die CacheEngine sollte eigentlich die 8 Schichten an
den PRT-ART vererben, die es an Algorithmus-Bausteinen gibt. Ich sehe
hier nicht klar aufgefuehrt, dass der PRT-ART strikt die vorhandenen
Strukturen und source Interfaces der CacheEngine erweitert."*

### V8.10 — prt-art-eigenes algorithm_profiles/

```
prt_art/algorithm_profiles/
├── README.md                       (Pruefling-Konzept-Erklaerung)
├── prtart_pruefling.profile.xml    (PRT-ART als Beitrittspruefling)
└── permutation_axes_extension.xml  (PRT-ART-spezifische Achsen-Erweiterung)
```

User-Direktive: *"In Messreihe A verwenden wir nur die Permutationen
fuer Suchalgorithmen, die bereits in der CacheEngine dokumentiert sind...
PRT-ART sollte mit neuen Layered Algorithmus-Baustein-Bestandteilen und
einer separaten eigenen Konfiguration im PRT-ART repo als Pruefling
dargestellt werden, um den Fall einer Beitrittspruefung eines
Algorithmus zum Stand der Technik zu zeigen."*

prt-art definiert ein **Pruefling-Profil**, das die SOTA-Profile in
cache-engine *erweitert* (Vererbung). Bei Beitritt zu SOTA wuerde dieses
Profil nach `cache-engine/algorithm_profiles/sota/` migrieren.

### V8.11 — TestDataSetAccumulationEngine im Constructor

```cpp
template <typename... Ts>
class PrtArtSearchEngine : public comdare::search_engine<...> {
public:
    explicit PrtArtSearchEngine(
        comdare::TestDataSetAccumulationEngine<PrtArtSearchEngine>& dataset_engine)
        : base_t{}, dataset_engine_{dataset_engine}
    {
        // dataset_engine ist beim Konstruktions-Zeitpunkt verfuegbar
    }

private:
    comdare::TestDataSetAccumulationEngine<PrtArtSearchEngine>& dataset_engine_;
};
```

User-Direktive (Master §10.11): *"In der Regel werden alle Testdatensaetze
als Implementierung der SearchEngine bei Initialisierung dieser eingelesen
test_data_set_accumulation_engine_type und bereitgestellt, um dann durch
eine separate Testroutine ausgefuehrt zu werden."*

---

## §2 Reihenfolge

V8.9 (ABI-Inheritance) zuerst (Foundation), dann V8.11 (Constructor-Param),
dann V8.10 (eigene algorithm_profiles).

Verifikation pro Schritt: cmake + ctest in prt-art-Repo (51 Tests in
test_prt_art_identity.cpp muessen weiter gruen sein).

---

## §3 Submodule-Pin-Bump cache-engine

prt-art aktuell: `external/comdare-cache-engine` zeigt auf `aa70a1c`.
Nach V8.4/V8.5 wird cache-engine neu commits haben — prt-art Submodule-
Pin muss auf den neuen HEAD (z.B. `b88eab1` oder neuer) gebumpt werden.

---

## §4 Akzeptanzkriterien (prt-art)

- [ ] `PrtArtSearchEngine` erbt von `comdare::search_engine` (Compile-Test)
- [ ] `prt_art/algorithm_profiles/prtart_pruefling.profile.xml` existiert
- [ ] `TestDataSetAccumulationEngine` ist im Konstruktor-Param verdrahtet
- [ ] cmake + ctest gruen (51 Tests + neue ABI-Compile-Tests)
- [ ] Submodule-Pin cache-engine auf neuen HEAD gebumpt
