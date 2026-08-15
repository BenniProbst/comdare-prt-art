// SPDX-License-Identifier: LicenseRef-Comdare-Research-1.0
// Copyright (c) 2026 BEP Venture UG (haftungsbeschraenkt), Marke Comdare
//
// test_lizenz_konsistenz -- die Wache ueber die prt-art-Lizenz-Umstellung. (2026-08-14)
// =============================================================================
// PORT der ce-Wache tests/unit/test_lizenz_konsistenz.cpp (2026-08-10, P6-Form).
// PRUEFLING: der Zustand dieses Repositoriums, nicht ein Skript.
//
// DER BEFUND, DER SIE AUSGELOEST HAT (am Objekt gemessen, 14.08.2026, prt-art c6f0754):
//   LICENSE  trug die VOLLE Apache-2.0 (4x "Apache License" im Volltext).
//   NOTICE   existierte NICHT (ls: "No such file or directory").
//   28 Quelldatei-Koepfe unter prt_art/legacy_reimpl/ trugen als SPDX-Bezeichner
//            weiter Apache-2.0.
//   README.md:81-83 behauptete "Apache-2.0 (analog comdare-cache-engine)" -- waehrend
//            die cache engine seit P6 (f6d13dfb) die Comdare Research License 1.0 traegt.
// Die 10.08.-Entscheidung (Owner 34851: "PRT-ART und cache eingine sollen beide frei
// fuer die Forschung und proprietaer fuer business und Einzelnutzung sein"; KON2-23
// "comdare-prt-art | dito") war hier VIER TAGE lang nicht umgesetzt, ohne jedes Signal.
// (Der volle fremde Bezeichner steht in dieser Datei NIRGENDS als Literal -- die
//  Koeder setzen ihn zur Laufzeit aus Teilen zusammen. Sonst zeigte die Wache beim
//  Lauf ueber den echten Baum ihre eigene Quelldatei an.)
//
// ENTFALLENE ce-FAELLE, BEGRUENDET (C3-Messung 14.08.2026 am Objekt):
//   L3 (NOTICE-Attribution eingezogener ext/-Komponenten): 0 add_subdirectory-Einzuege
//      in Fremdverzeichnisse -- prt-art hat kein ext/. GoogleTest kommt als reines
//      Test-Werkzeug per FetchContent und steht im NOTICE.
//   L4 (Copyleft-Schalter): 0 Copyleft-Schalter; einzige option()s sind COMDARE_PRT_ART_*.
//   L5 (GPL-Kopie wh.c): kein wh.c, kein COMDARE_CE_ENABLE_ORIGINAL_CODE_VALIDATION.
//   L6 (COMDARE_WRITER_BACKEND): existiert in prt-art nicht.
// Bekommt prt-art je ein ext/ oder einen Copyleft-Schalter, sind diese Faelle aus dem
// ce-Original nachzuziehen -- die Erhebungen dort sind fertig.
//
// WAS DIESE WACHE KANN, UND WAS NICHT (T-9, ehrlich):
//   SIE KANN     : dass alle Lizenz-Aussagen DIESES Baums denselben Bezeichner nennen.
//   SIE KANN NICHT: beurteilen, ob die Lizenz rechtlich traegt. Eine Umstellung mit
//                  Wirkung fuer Dritte gehoert vor einen Anwalt (KON2-24) -- besonders
//                  der unwiderrufliche Apache-Grant fuer Revisionen 02.08.-09.08.2026.
//
// K13: jeder Koeder wird aus /dev/urandom gewuerfelt und beidseitig gefahren --
// mit Koeder ROT, ohne Koeder GRUEN (Gegeneingang Fall 5).
//
// MUTATIONS-NACHWEIS (T-1): COMDARE_LIZENZ_WURZEL schiebt einen anderen Baum
// unter; welcher gefahren wurde, steht in der Ausgabe jedes Falls.
//
// ASCII-only, Zeilen <= 120 Byte.
// =============================================================================

#include <gtest/gtest.h>

#include "support/lizenz_audit.hpp"
#include "support/wachen_werkbank.hpp"

#include <cstdlib>
#include <string>

namespace {

namespace fs = std::filesystem;
using comdare::prt_art::test::lizenz::ChangeDateErnte;
using comdare::prt_art::test::lizenz::ernte_change_date;
using comdare::prt_art::test::lizenz::ernte_spdx;
using comdare::prt_art::test::lizenz::kBezeichner;
using comdare::prt_art::test::lizenz::kChangeDate;
using comdare::prt_art::test::lizenz::kPlatzhalterOf1;
using comdare::prt_art::test::lizenz::lies_datei;
using comdare::prt_art::test::lizenz::SpdxErnte;
using comdare::prt_art::test::wachen::koeder;
using comdare::prt_art::test::wachen::WegwerfRepo;

[[nodiscard]] fs::path wurzel() {
    if (char const* const ueberschrieben = std::getenv("COMDARE_LIZENZ_WURZEL")) {
        if (*ueberschrieben != '\0') { return fs::path{ueberschrieben}; }
    }
    return fs::path{COMDARE_LIZENZ_REPO_WURZEL};
}

[[nodiscard]] bool enthaelt(std::string const& heu, std::string_view nadel) {
    return heu.find(nadel) != std::string::npos;
}

// Der fremde Bezeichner, zur Laufzeit zusammengesetzt. Als Literal wuerde er in dieser
// Datei stehen -- und die Wache liest die ganze Datei, nicht nur ihren Kopf. Sie wuerde
// sich selbst als Verstoss melden. Zusammensetzen ist hier kein Trick, sondern die
// einzige Form, in der ein Koeder seinen eigenen Pruefling nicht vergiftet.
[[nodiscard]] std::string fremder_bezeichner() { return std::string{"Apache"} + "-2.0"; }
[[nodiscard]] std::string fremder_kopf() {
    return "// " + std::string{"SPDX-License-"} + "Identifier: " + fremder_bezeichner() + "\n";
}

// Ein minimaler, in sich stimmiger Baum. Er ist der GEGENEINGANG jedes Koeders:
// erst muss er gruen sein, dann darf ein Koeder ihn rot machen. Ohne diesen
// Schritt bewiese ein roter Koeder-Lauf nur, dass die Wache ueberhaupt meckert.
void baue_sauberen_baum(WegwerfRepo const& repo, std::string const& marke) {
    ASSERT_TRUE(repo.schreibe("LICENSE", "Comdare Research License, Version 1.0\n"
                                         "SPDX-License-Identifier: " +
                                             std::string{kBezeichner} +
                                             "\n"
                                             "Change Date: " +
                                             std::string{kChangeDate} +
                                             "\n"
                                             "marke_" +
                                             marke + "\n"));
    ASSERT_TRUE(repo.schreibe("NOTICE", "SPDX-License-Identifier: " + std::string{kBezeichner} + "\n"));
    ASSERT_TRUE(repo.schreibe("prt_art/echt.hpp", "// SPDX-License-Identifier: " + std::string{kBezeichner} + "\n"));
}

} // namespace

// =============================================================================
// FALL 1 -- L1: EIN Repo, EIN Lizenz-Bezeichner.
// Das ist der Fall, der am 14.08. rot war: 28 Koepfe sagten Apache-2.0.
// =============================================================================
TEST(LizenzKonsistenz, KeinFremderSpdxBezeichnerImEigencode) {
    SpdxErnte const e = ernte_spdx(wurzel());

    std::string liste;
    for (auto const& f : e.fremd) { liste += "\n    " + f.datei + ":" + std::to_string(f.zeile) + "  " + f.text; }
    std::string doku;
    for (auto const& f : e.doku_ausnahme) {
        doku += "\n    " + f.datei + ":" + std::to_string(f.zeile) + "  " + f.text;
    }

    // Die beiden Nenner zaehlen VERSCHIEDENE Dinge, und das muss dastehen: 'Dateien'
    // sind Dateien, die Zeilen darunter sind VORKOMMEN. Eine Datei kann den
    // Bezeichner mehrfach tragen. Wer beides als eine Zahl liest, findet eine
    // Differenz, die keine ist.
    std::cout << "NENNER L1  wurzel=" << wurzel().string() << "\n"
              << "  Dateien durchsucht          : " << e.dateien_gesehen << "\n"
              << "  Dateien mit SPDX-Zeile      : " << e.dateien_mit_spdx << "\n"
              << "  Vorkommen eigener Bezeichner: " << e.eigen << " (" << kBezeichner << ")\n"
              << "  Vorkommen fremder Bezeichner: " << e.fremd.size() << " (SOLL 0)" << liste << "\n"
              << "  Vorkommen Doku-Ausnahme     : " << e.doku_ausnahme.size() << " unter docs/" << doku << "\n";

    // Nenner am Objekt ausgezaehlt: VOR dem Wachen-Add (14.08.2026, Apache-IST) 136
    // Kandidaten-Dateien, 28 Eigencode-SPDX-Koepfe; MIT der Wache 139 und 29 (dazu
    // kamen diese cpp und 2 support-hpp, einen SPDX-Kopf traegt nur die cpp).
    // Schwellen mit Abstand darunter -- gegen die stille Null, nicht als exakte
    // Buchhaltung.
    EXPECT_GT(e.dateien_gesehen, 60U) << "zu wenige Dateien gesehen -- die Wache lief ins Leere (stille Null)";
    EXPECT_GT(e.eigen, 20U) << "der eigene Bezeichner kommt fast nicht vor -- Erhebung unglaubwuerdig";
    EXPECT_TRUE(e.fremd.empty()) << "Dateien behaupten eine Lizenz, die nicht mehr gilt:" << liste;
}

// =============================================================================
// FALL 2 -- L2: LICENSE und NOTICE sagen dasselbe.
// Bei prt-art war NOTICE am 14.08. nicht bloss widerspruechlich, sondern ABWESEND --
// lies_datei liefert dann leer, und die Bezeichner-Pruefung (b) schlaegt an.
// =============================================================================
TEST(LizenzKonsistenz, LicenseUndNoticeNennenDenselbenBezeichner) {
    std::string const lic  = lies_datei(wurzel() / "LICENSE");
    std::string const not_ = lies_datei(wurzel() / "NOTICE");

    int        erfuellt = 0;
    bool const a        = enthaelt(lic, kBezeichner);
    bool const b        = enthaelt(not_, kBezeichner);
    bool const c        = !enthaelt(not_, "Licensed under the Apache License");
    erfuellt            = static_cast<int>(a) + static_cast<int>(b) + static_cast<int>(c);

    std::cout << "NENNER L2  pruefungen " << erfuellt << "/3\n"
              << "  LICENSE nennt Bezeichner            : " << (a ? "ja" : "NEIN") << "\n"
              << "  NOTICE  nennt Bezeichner            : " << (b ? "ja" : "NEIN") << "\n"
              << "  NOTICE  behauptet NICHT mehr Apache : " << (c ? "ja" : "NEIN") << "\n";

    EXPECT_TRUE(a) << "LICENSE traegt den Bezeichner nicht";
    EXPECT_TRUE(b) << "NOTICE traegt den Bezeichner nicht (oder fehlt -- der 14.08.-Befund)";
    EXPECT_TRUE(c) << "NOTICE behauptet weiter Apache-2.0 -- die Widerspruchs-Klasse vom 10.08.2026 (ce)";
}

// =============================================================================
// FALL 3 -- L7: Change-Date-Feld, Ziel-Lizenz und die WAHRE prt-art-Historie.
// Historie am Objekt gemessen (C1, git log --follow -- LICENSE): die LICENSE-Datei
// wurde erst am 02.08.2026 angelegt (7f6c7032, "Datei fehlte") und war seither
// durchgehend Apache-2.0 -- prt-art hatte NIE die ce-Dual-Phase. Beide
// Historie-Daten (2026-08-02 Apache-Beginn, 2026-08-10 Entscheidtag) muessen stehen.
// =============================================================================
TEST(LizenzKonsistenz, ChangeDateStehtUndIstDerEntschiedene) {
    std::string const lic    = lies_datei(wurzel() / "LICENSE");
    bool const        datum  = enthaelt(lic, kChangeDate);
    bool const        feld   = enthaelt(lic, "Change Date");
    bool const        ziel   = enthaelt(lic, "Apache License, Version 2.0");
    bool const        hist_a = enthaelt(lic, "2026-08-02");
    bool const        hist_b = enthaelt(lic, "2026-08-10");

    std::cout << "NENNER L7  pruefungen "
              << (static_cast<int>(datum) + static_cast<int>(feld) + static_cast<int>(ziel) + static_cast<int>(hist_a) +
                  static_cast<int>(hist_b))
              << "/5\n"
              << "  Change Date " << kChangeDate << " : " << (datum ? "ja" : "NEIN") << "\n"
              << "  Feld 'Change Date'      : " << (feld ? "ja" : "NEIN") << "\n"
              << "  Ziel-Lizenz Apache-2.0  : " << (ziel ? "ja" : "NEIN") << "\n"
              << "  Historie 2026-08-02     : " << (hist_a ? "ja" : "NEIN") << "\n"
              << "  Historie 2026-08-10     : " << (hist_b ? "ja" : "NEIN") << "\n";

    EXPECT_TRUE(datum) << "der Change Date (kChangeDate) fehlt in der LICENSE";
    EXPECT_TRUE(feld) << "kein maschinenlesbares Feld 'Change Date'";
    EXPECT_TRUE(ziel) << "die Lizenz nach dem Change Date ist nicht benannt";
    EXPECT_TRUE(hist_a && hist_b) << "die Lizenz-Historie nennt nicht beide Daten -- alte Grants bleiben gueltig";
}

// =============================================================================
// FALL 4 -- L7b: das Change Date ist ein ECHTES ISO-Datum (JJJJ-MM-TT).
// DIESER FALL IST DAS OF-1-GATE: das Datum ist owner-woertlich nur fuer die cache
// engine entschieden (C-2); fuer prt-art steht die Owner-Antwort OF-1 aus. Solange
// kChangeDate den Platzhalter OF1-OFFEN traegt, ist dieser Fall LAUT ROT -- ein Baum
// mit Platzhalter darf NIE landen. Nach dem Einsetzen der Owner-Antwort ist der Fall
// dauerhaft die Rueckfall-Sperre gegen Platzhalter-Regressionen.
// =============================================================================
TEST(LizenzKonsistenz, ChangeDateIstEinIsoDatumNachOf1) {
    std::string const d   = std::string{kChangeDate};
    bool              iso = (d.size() == 10) && (d[4] == '-') && (d[7] == '-');
    if (iso) {
        for (std::size_t i : {0U, 1U, 2U, 3U, 5U, 6U, 8U, 9U}) {
            if (d[i] < '0' || d[i] > '9') { iso = false; }
        }
    }

    std::cout << "NENNER L7b kChangeDate='" << d << "'  iso=" << (iso ? "ja" : "NEIN (OF-1 offen)") << "\n";

    EXPECT_TRUE(iso) << "kChangeDate='" << d << "' ist kein ISO-Datum -- Owner-Frage OF-1 (Change Date fuer "
                     << "prt-art) ist offen; mit Platzhalter darf dieser Baum NICHT gemergt werden";
}

// =============================================================================
// FALL 4b -- L7c: das Change Date steht in ALLEN drei Aussagen-Dateien, und nach
// der OF-1-Antwort bleibt KEIN Platzhalter-Rest zurueck.
// F1-NACHTRAG (Review 14.08.2026): L7 las nur die LICENSE, L7b nur die Konstante.
// Ein nach OF-1 vergessener Platzhalter in NOTICE (Z.15) oder README.md (Z.95)
// bliebe wachen-gruen -- die 15794-Klasse (LICENSE-NOTICE-Divergenz), die diese
// Wache decken soll; am 14.08. beidseitig gemessen: der Divergenz-Baum (NOTICE und
// README.md ohne Datum) passierte L2+L7 GRUEN. Die LICENSE steht mit im
// Platzhalter-Radar: sie traegt den Platzhalter VIERMAL (Z.12/34/92/237), und L7
// ("enthaelt kChangeDate") saehe drei vergessene von vier Stellen nicht.
// Kommentar-Erwaehnungen in lizenz_audit.hpp sind bewusst KEIN Gegenstand: dort
// sind sie Historie; die Konstante selbst prueft L7b.
// =============================================================================
TEST(LizenzKonsistenz, ChangeDateStehtInNoticeUndReadmeOhnePlatzhalterRest) {
    ChangeDateErnte const e         = ernte_change_date(wurzel(), kChangeDate);
    bool const            of1_offen = (kChangeDate == kPlatzhalterOf1);

    std::string funde;
    for (auto const& f : e.platzhalter) { funde += "\n    " + f.datei + ":" + std::to_string(f.zeile) + "  " + f.text; }

    std::cout << "NENNER L7c wurzel=" << wurzel().string() << "  of1_offen=" << (of1_offen ? "ja" : "nein") << "\n"
              << "  LICENSE   traegt kChangeDate: " << (e.license_traegt_datum ? "ja" : "NEIN") << "\n"
              << "  NOTICE    traegt kChangeDate: " << (e.notice_traegt_datum ? "ja" : "NEIN") << "\n"
              << "  README.md traegt kChangeDate: " << (e.readme_traegt_datum ? "ja" : "NEIN") << "\n"
              << "  Platzhalter-Zeilen in LICENSE/NOTICE/README.md: " << e.platzhalter.size()
              << (of1_offen ? " (solange OF-1 offen: die legitimen Traeger)" : " (SOLL 0)") << funde << "\n";

    EXPECT_TRUE(e.license_traegt_datum) << "LICENSE traegt kChangeDate nicht";
    EXPECT_TRUE(e.notice_traegt_datum)
        << "NOTICE traegt kChangeDate nicht -- LICENSE und NOTICE laufen auseinander (15794-Klasse)";
    EXPECT_TRUE(e.readme_traegt_datum) << "README.md traegt kChangeDate nicht -- die Nutzer-Sicht nennt kein Datum";
    if (of1_offen) {
        EXPECT_FALSE(e.platzhalter.empty())
            << "OF-1 offen, aber keine der drei Dateien nennt den Platzhalter -- Trio und Konstante divergieren";
    } else {
        EXPECT_TRUE(e.platzhalter.empty()) << "kChangeDate ist entschieden ('" << kChangeDate << "'), aber der "
                                           << "Platzhalter " << kPlatzhalterOf1 << " steht noch:" << funde;
    }
}

// =============================================================================
// FALL 5 -- DER GEGENEINGANG. Ohne ihn beweisen die Koeder unten nichts.
// =============================================================================
TEST(LizenzKonsistenzKoeder, SaubererBaumIstGruen) {
    std::string const marke = koeder();
    WegwerfRepo const repo{"lizenz_sauber_" + marke};
    baue_sauberen_baum(repo, marke);

    SpdxErnte const   e     = ernte_spdx(repo.pfad());
    std::string const notiz = lies_datei(repo.pfad() / "NOTICE");

    std::cout << "GEGENEINGANG marke=" << marke << "  fremd=" << e.fremd.size() << " eigen=" << e.eigen
              << " notice_da=" << (notiz.empty() ? "NEIN" : "ja") << "\n";

    EXPECT_TRUE(e.fremd.empty()) << "der saubere Baum ist schon ohne Koeder rot -- die Wache meckert grundlos";
    EXPECT_EQ(e.eigen, 1U) << "genau der eine Quelldatei-Kopf muss als eigen zaehlen";
    EXPECT_FALSE(notiz.empty()) << "NOTICE des Wegwerf-Baums fehlt -- das Arrangement traegt nicht";
}

// =============================================================================
// FALL 6 -- KOEDER A: eine Datei behauptet Apache-2.0. L1 MUSS beissen.
// Das ist der Koeder auf den 14.08.-Befund selbst (28 solcher Koepfe im Bestand).
// =============================================================================
TEST(LizenzKonsistenzKoeder, FremderSpdxBezeichnerWirdGefunden) {
    std::string const marke = koeder();
    WegwerfRepo const repo{"lizenz_koeder_spdx_" + marke};
    baue_sauberen_baum(repo, marke);

    std::string const pfad = "prt_art/koeder_" + marke + ".hpp";
    ASSERT_TRUE(repo.schreibe(pfad, fremder_kopf()));

    SpdxErnte const e = ernte_spdx(repo.pfad());
    std::string     gefunden;
    for (auto const& f : e.fremd) { gefunden += "\n    " + f.datei + ":" + std::to_string(f.zeile) + "  " + f.text; }

    std::cout << "KOEDER A marke=" << marke << "  erwartet 1 Fund, gefunden " << e.fremd.size() << gefunden << "\n";

    ASSERT_EQ(e.fremd.size(), 1U) << "der Koeder wurde nicht gefunden -- die Wache ist blind";
    EXPECT_EQ(e.fremd[0].datei, pfad) << "gefunden wurde etwas anderes als der gewuerfelte Koeder";
    EXPECT_EQ(e.fremd[0].text, fremder_bezeichner());
}

// =============================================================================
// FALL 7 -- KOEDER B: NOTICE behauptet wieder Apache. L2 MUSS beissen.
// (ce-Koeder E; die Schalter-/Vendor-Koeder B-D des Originals entfallen mit L3-L6.)
// =============================================================================
TEST(LizenzKonsistenzKoeder, NoticeMitApacheBehauptungWirdGefunden) {
    std::string const marke = koeder();
    WegwerfRepo const repo{"lizenz_koeder_apache_" + marke};
    baue_sauberen_baum(repo, marke);

    ASSERT_TRUE(repo.schreibe("NOTICE", "Licensed under the Apache License, Version 2.0\nmarke_" + marke + "\n"));

    std::string const notiz  = lies_datei(repo.pfad() / "NOTICE");
    bool const        sauber = !enthaelt(notiz, "Licensed under the Apache License");

    std::cout << "KOEDER B marke=" << marke << "  NOTICE-behauptet-Apache=" << (sauber ? "nein" : "ja") << "\n";

    EXPECT_FALSE(sauber) << "die Apache-Behauptung im NOTICE blieb unbemerkt -- Fall 2 ist wirkungslos";
    EXPECT_FALSE(enthaelt(notiz, kBezeichner)) << "Gegenprobe: der Koeder hat den Bezeichner wirklich verdraengt";
}

// =============================================================================
// FALL 8 -- KOEDER C (prt-art-eigen, F1-Nachtrag): die Nach-OF-1-Welt. Ein Baum
// mit ISO-Datum in allen drei Dateien ist gruen (Gegeneingang); dann 'vergisst'
// der Koeder GENAU die NOTICE-Ersetzung -- die L7c-Erhebung MUSS auf beiden
// Zinken beissen: Datum fehlt UND Platzhalter-Rest gefunden. kChangeDate bleibt
// unberuehrt; die Ernte faehrt das Datum des Wegwerf-Baums als Parameter. Damit
// ist der (bis OF-1 tote) else-Zweig von Fall 4b in seiner Logik beidseitig
// bewiesen, ohne die Konstante anzufassen.
// =============================================================================
TEST(LizenzKonsistenzKoeder, VergessenerOf1PlatzhalterInNoticeWirdGefunden) {
    std::string const marke = koeder();
    WegwerfRepo const repo{"lizenz_koeder_of1_" + marke};
    std::string const iso = "2031-01-01"; // stellvertretendes Nach-OF-1-Datum, existiert nur im Wegwerf-Baum

    ASSERT_TRUE(repo.schreibe("LICENSE", "Change Date: " + iso + "\nmarke_" + marke + "\n"));
    ASSERT_TRUE(repo.schreibe("NOTICE", "On the Change Date " + iso + " ...\n"));
    ASSERT_TRUE(repo.schreibe("README.md", "| ab " + iso + " |\n"));

    ChangeDateErnte const sauber = ernte_change_date(repo.pfad(), iso);
    ASSERT_TRUE(sauber.license_traegt_datum && sauber.notice_traegt_datum && sauber.readme_traegt_datum)
        << "GEGENEINGANG: der saubere Nach-OF-1-Baum traegt das Datum nicht in allen drei Dateien";
    ASSERT_TRUE(sauber.platzhalter.empty()) << "GEGENEINGANG: der saubere Baum meldet schon einen Platzhalter";

    // Der Koeder: die NOTICE wird 'vergessen' -- Platzhalter statt Datum, mit Marke.
    ASSERT_TRUE(
        repo.schreibe("NOTICE", "On the Change Date " + std::string{kPlatzhalterOf1} + " marke_" + marke + "\n"));

    ChangeDateErnte const e = ernte_change_date(repo.pfad(), iso);
    std::string           funde;
    for (auto const& f : e.platzhalter) { funde += "\n    " + f.datei + ":" + std::to_string(f.zeile) + "  " + f.text; }
    std::cout << "KOEDER C marke=" << marke << "  notice_traegt_datum=" << (e.notice_traegt_datum ? "ja" : "NEIN")
              << "  platzhalter=" << e.platzhalter.size() << funde << "\n";

    EXPECT_FALSE(e.notice_traegt_datum) << "die vergessene NOTICE gilt als datiert -- Zinke 1 ist stumpf";
    ASSERT_EQ(e.platzhalter.size(), 1U) << "der Platzhalter-Rest wurde nicht (oder mehrfach) gefunden" << funde;
    EXPECT_EQ(e.platzhalter[0].datei, "NOTICE") << "der Fund liegt nicht in der praeparierten NOTICE";
    EXPECT_TRUE(e.platzhalter[0].text.find("marke_" + marke) != std::string::npos)
        << "der Fund traegt die gewuerfelte Marke nicht -- er stammt nicht aus diesem Wegwerf-Baum";
}
