// lizenz_audit.hpp -- die reinen Erhebungen der prt-art-Lizenz-Wache.      (2026-08-14)
// =============================================================================
// PORT aus comdare-cache-engine tests/unit/support/lizenz_audit.hpp (2026-08-10,
// P6-Form). WARUM ES DIESE DATEI GIBT: Ein Lizenzwechsel ist kein einmaliger
// Textvorgang, sondern ein Zustand, der auseinanderlaufen kann -- und der genau
// dann auseinanderlaeuft, wenn niemand misst. Am 14.08.2026 stand genau das in
// DIESEM Repo: die 10.08.-Entscheidung (KON2-23, "PRT-ART ... dito") war fuer die
// cache engine umgesetzt (P6, f6d13dfb), waehrend prt-art am Objekt weiter die
// volle Apache-2.0-LICENSE, KEIN NOTICE und 28 Apache-SPDX-Koepfe trug.
//
// TRENNUNG wie im ce-Original: hier steht NUR das Erheben (reine Funktionen ueber
// einem Wurzelpfad), test_lizenz_konsistenz.cpp faellt das Urteil. Nur so kann der
// Koeder dieselbe Funktion ueber einen praeparierten Wegwerf-Baum fahren wie ueber
// den echten -- ein Orakel, das nur im echten Baum laufen kann, ist nicht widerlegbar.
//
// PRT-ART-ABWEICHUNGEN VOM ce-ORIGINAL (beide am Objekt gemessen, 14.08.2026):
//   (a) KEIN ext/-Skip: prt-art buendelt keinen Fremdcode (Wurzel-Listing ohne ext/;
//       C3-Messung: 0 add_subdirectory-Einzuege in Fremdverzeichnisse). Entsteht je
//       ein ext/, ist die Skip-Entscheidung hier NEU zu treffen -- nicht stillschweigend.
//   (b) KEINE Schalter-/Vendor-Ernten: 0 Copyleft-Schalter (einzige option()s sind
//       COMDARE_PRT_ART_*), kein COMDARE_WRITER_BACKEND, kein wh.c. Die ce-Faelle
//       L3/L4/L5/L6 haben hier keinen Gegenstand und entfallen BEGRUENDET.
//
// ASCII-only, Zeilen <= 120 Byte.
// =============================================================================
#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace comdare::prt_art::test::lizenz {

namespace fs = std::filesystem;

// Der EINE Bezeichner. Er steht in LICENSE, in NOTICE und in jedem Quelldatei-Kopf.
// Genau deshalb ist er hier eine einzige Konstante: drei Orte, eine Wahrheit.
inline constexpr std::string_view kBezeichner = "LicenseRef-Comdare-Research-1.0";

// OWNER-FRAGE OF-1 -- BEANTWORTET (OF-1-GO, 15.08.2026): Change Date 2031-08-10, wie
// die cache engine (C-2, 10.08.2026: "Ja, cache engine nach 5 Jahren frei verfuegbar
// ab heute" -> 2031-08-10). Historie der Frage: das KON2-23-"dito" deckte die
// Dreiteilung frei-Forschung/proprietaer-Business+Einzelnutzung, nicht ausdruecklich
// das Datum; bis zum GO trug dieses Feld den Platzhalter (kPlatzhalterOf1) und der
// Fall ChangeDateIstEinIsoDatumNachOf1 hielt den Baum LAUT ROT. Mit dem GO ist der
// Platzhalter in LICENSE, NOTICE, README.md und HIER ersetzt; L7c sperrt Reste.
inline constexpr std::string_view kChangeDate = "2031-08-10";

// Der OF-1-Platzhalter als EIGENE Konstante neben kChangeDate: solange OF-1 offen
// ist, sind beide identisch; mit der Owner-Antwort trennt sich das Paar, und genau
// diese Trennung schaltet die Platzhalter-Rest-Sperre (L7c) scharf. F1-Nachtrag
// (Review 14.08.2026): L7 las nur die LICENSE, L7b nur die Konstante -- ein nach
// OF-1 vergessener Platzhalter in NOTICE/README.md blieb wachen-gruen (die
// 15794-Klasse: LICENSE-NOTICE-Divergenz).
inline constexpr std::string_view kPlatzhalterOf1 = "OF1-OFFEN";

struct Fund {
    std::string datei;
    std::size_t zeile = 0;
    std::string text;
};

// -----------------------------------------------------------------------------
// SPDX-ERNTE -- welche Lizenz behaupten die Dateien selbst?
// -----------------------------------------------------------------------------
struct SpdxErnte {
    std::size_t       dateien_gesehen  = 0; // Nenner: durchsuchte Kandidaten
    std::size_t       dateien_mit_spdx = 0;
    std::size_t       eigen            = 0; // traegt kBezeichner
    std::vector<Fund> fremd;                // traegt etwas anderes  -> Verstoss
    std::vector<Fund> doku_ausnahme;        // dasselbe, aber unter docs/ -> benannt, kein Verstoss
};

namespace detail {

[[nodiscard]] inline bool beginnt_mit(std::string const& s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

// Welche Verzeichnisse die Wache NICHT betritt -- und warum jedes einzelne:
//   .git/      keine Quelle.
//   build*/    Erzeugnisse; ihre Koepfe stammen aus den Vorlagen, die geprueft werden.
//   .idea/     Werkzeugkonfiguration.
// KEIN ext/-Skip (Abweichung (a) im Dateikopf): prt-art buendelt keinen Fremdcode.
//
// DIE FALLE AUS DEM ce-ORIGINAL, HIER NICHT WIEDERHOLT: `beginnt_mit(name, "build")`
// fraesse ein kuenftiges `builder/` mit (FALLEN-REGISTER: "`grep -v /build` frisst
// `/builder/`"). Deshalb: EXAKTE Namen und ein Trenner, nie ein blosses Praefix.
[[nodiscard]] inline bool ist_uebersprungenes_verzeichnis(std::string const& name) {
    return name == ".git" || name == ".idea" || name == "node_modules" || name == "build" ||
           beginnt_mit(name, "build-") || beginnt_mit(name, "build_") || beginnt_mit(name, "cmake-build");
}

[[nodiscard]] inline bool ist_kandidat(fs::path const& p) {
    if (p.filename() == "CMakeLists.txt") { return true; }
    std::string const                  e = p.extension().string();
    static std::set<std::string> const kEndungen{".c",  ".cc", ".cxx",   ".cpp", ".h",    ".hpp", ".hxx", ".in",
                                                 ".py", ".sh", ".cmake", ".yml", ".yaml", ".md",  ".txt"};
    return kEndungen.count(e) != 0;
}

[[nodiscard]] inline std::string lies(fs::path const& p) {
    std::ifstream ein{p, std::ios::binary};
    if (!ein.good()) { return {}; }
    std::ostringstream puffer;
    puffer << ein.rdbuf();
    return puffer.str();
}

// WARUM DIE GANZE DATEI GELESEN WIRD UND NICHT NUR DER KOPF (ce-Lehre): eine Wache,
// die nur Koepfe liest, uebersieht Vorlagen, aus denen neue Dateien gestempelt
// werden, und raeumt ewig hinterher. Der Preis ist, dass auch Erwaehnungen im
// Fliesstext zaehlen -- das ist gewollt, denn in Code- und Bau-Dateien gibt es
// keinen harmlosen Ort fuer eine fremde Lizenz-Behauptung. Fuer Dokumentation
// (docs/) gilt die benannte Ausnahme.
} // namespace detail

[[nodiscard]] inline SpdxErnte ernte_spdx(fs::path const& wurzel) {
    SpdxErnte              ernte;
    std::string_view const marke = "SPDX-License-Identifier:";

    std::error_code                  ec;
    fs::recursive_directory_iterator it{wurzel, fs::directory_options::skip_permission_denied, ec};
    fs::recursive_directory_iterator ende;
    for (; it != ende; it.increment(ec)) {
        if (ec) { break; }
        if (it->is_directory(ec)) {
            if (detail::ist_uebersprungenes_verzeichnis(it->path().filename().string())) {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (!it->is_regular_file(ec)) { continue; }
        if (!detail::ist_kandidat(it->path())) { continue; }

        ernte.dateien_gesehen++;
        std::string const inhalt = detail::lies(it->path());
        if (inhalt.find(marke) == std::string::npos) { continue; }

        std::string const rel  = fs::relative(it->path(), wurzel, ec).generic_string();
        bool const        doku = detail::beginnt_mit(rel, "docs/");

        std::istringstream zeilen{inhalt};
        std::string        zeile;
        std::size_t        nr       = 0;
        bool               gezaehlt = false;
        while (std::getline(zeilen, zeile)) {
            nr++;
            std::size_t const pos = zeile.find(marke);
            if (pos == std::string::npos) { continue; }
            std::string wert = zeile.substr(pos + marke.size());
            // fuehrende Leerzeichen und ein evtl. schliessendes Anfuehrungszeichen weg
            std::size_t const erst = wert.find_first_not_of(" \t");
            wert.erase(0, erst == std::string::npos ? wert.size() : erst);
            std::size_t const schluss = wert.find_first_of(" \t\"'*/");
            if (schluss != std::string::npos) { wert.erase(schluss); }
            // Die Marke OHNE Wert ist keine Lizenz-Behauptung, sondern ein Vorkommen der
            // Marke selbst -- z.B. in dieser Datei, die nach ihr sucht. Wer das nicht
            // ausnimmt, baut eine Wache, die sich selbst anzeigt.
            if (wert.empty()) { continue; }
            if (!gezaehlt) {
                ernte.dateien_mit_spdx++;
                gezaehlt = true;
            }
            if (wert == kBezeichner) {
                ernte.eigen++;
            } else if (doku) {
                ernte.doku_ausnahme.push_back({rel, nr, wert});
            } else {
                ernte.fremd.push_back({rel, nr, wert});
            }
        }
    }
    return ernte;
}

// -----------------------------------------------------------------------------
// CHANGE-DATE-ERNTE -- tragen LICENSE, NOTICE und README.md das Datum, und wo
// steht der OF-1-Platzhalter noch? Nur Erhebung; das Urteil (L7c) faellt
// test_lizenz_konsistenz.cpp. Das Datum ist PARAMETER, kein Durchgriff auf
// kChangeDate: nur so kann der Koeder dieselbe Funktion mit einem ISO-Datum
// ueber einen Wegwerf-Baum fahren, ohne die Konstante anzufassen.
// -----------------------------------------------------------------------------
struct ChangeDateErnte {
    bool              license_traegt_datum = false;
    bool              notice_traegt_datum  = false;
    bool              readme_traegt_datum  = false;
    std::vector<Fund> platzhalter; // jede Zeile mit kPlatzhalterOf1 in den drei Dateien
};

[[nodiscard]] inline ChangeDateErnte ernte_change_date(fs::path const& wurzel, std::string_view datum) {
    ChangeDateErnte ernte;
    auto const      ernte_datei = [&](char const* datei, bool& traegt) {
        std::string const inhalt = detail::lies(wurzel / datei);
        traegt                   = !datum.empty() && inhalt.find(datum) != std::string::npos;
        std::istringstream zeilen{inhalt};
        std::string        zeile;
        std::size_t        nr = 0;
        while (std::getline(zeilen, zeile)) {
            nr++;
            if (zeile.find(kPlatzhalterOf1) != std::string::npos) {
                ernte.platzhalter.push_back({datei, nr, zeile});
            }
        }
    };
    ernte_datei("LICENSE", ernte.license_traegt_datum);
    ernte_datei("NOTICE", ernte.notice_traegt_datum);
    ernte_datei("README.md", ernte.readme_traegt_datum);
    return ernte;
}

[[nodiscard]] inline std::string lies_datei(fs::path const& p) { return detail::lies(p); }

} // namespace comdare::prt_art::test::lizenz
