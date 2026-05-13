# prt_art/algorithm_profiles/ — PRT-ART Pruefling-Profil (REV 7.6 V8.10)

**Anlass:** User-Direktive 2026-05-13/14
(`Diplomarbeit/STRUCTURAL_CORRECTION_diplomarbeit.md` §2.2):

> *"PRT-ART sollte mit neuen Layered Algorithmus-Baustein-Bestandteilen
> und einer separaten eigenen Konfiguration im PRT-ART repo als Pruefling
> dargestellt werden, um den Fall einer Beitrittspruefung eines Algorithmus
> zum Stand der Technik zu zeigen."*

---

## Zweck

prt-art ist der **Pruefling**, der Beitrittsfaehigkeit zum Stand der
Technik (cache-engine SOTA) demonstriert. Diese Konfiguration hier
beschreibt PRT-ART als kompletten Algorithmus mit allen 11 Achsen — analog
zu den `cache-engine/algorithm_profiles/sota/`-Profilen.

Bei erfolgreicher Beitrittspruefung wird `prtart_pruefling.profile.xml`
nach `cache-engine/algorithm_profiles/sota/prtart.profile.xml` migriert.

---

## Layout

```
prt_art/algorithm_profiles/
├── README.md                       (dieses Dokument)
├── prtart_pruefling.profile.xml    (PRT-ART als Beitrittspruefling)
└── permutation_axes_extension.xml  (PRT-ART-spezifische Achsen-Erweiterung)
```

---

## Querverweis

- Diplomarbeit: `STRUCTURAL_CORRECTION_diplomarbeit.md` §2.2
- cache-engine: `cache_engine/algorithm_profiles/README.md` (SOTA-Profile)
- Diplomarbeit-Session: `docs/sessions/20260514-0900-v8-implementations-anker-rev7-6.md`
- prt-art-Session: `docs/sessions/20260514-0900-v8-prt-art-abi-inheritance.md`
