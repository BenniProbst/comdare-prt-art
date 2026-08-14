// wachen_werkbank.hpp -- prt-art-Teilmenge der ce-Wachen-Werkbank.         (2026-08-14)
// =============================================================================
// PORT aus comdare-cache-engine tests/unit/support/wachen_werkbank.hpp (2026-08-10,
// P6-Form). Uebernommen ist NUR, was die prt-art-Lizenzwache braucht:
//
//  (1) DER KOEDER-WUERFEL (K13). Kein Orakel schreibt eine Kennung ab. Jede Marke
//      kommt frisch aus /dev/urandom und wird WOERTLICH in der Ausgabe der Wache
//      zurueckgefordert. Steht sie dort, kann sie nur aus dem Gegenstand stammen,
//      den dieser Fall gerade angelegt hat -- aus keiner Datei dieses Repos.
//
//  (2) EIN WEGWERF-GEGENSTAND als reiner Dateibaum. Die ce-Werkbank baut zusaetzlich
//      echte git-Wegwerf-Repos, weil zwei ce-Wachen `git` befragen; die prt-art-
//      Lizenzwache befragt nur das Dateisystem, also entfaellt der git-Teil hier
//      BEGRUENDET (Teilmenge, kein Eigenbau).
//
// Temp-Wurzel uid-suffixiert (ce-Posten #278/#24: feste Namen unter /tmp kollidieren
// auf Shell-Runnern mit Resten FREMDER User; ofstream/remove scheitern dann STILL).
//
// ASCII-only, Zeilen <= 120 Byte.
// =============================================================================
#pragma once

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace comdare::prt_art::test::wachen {

namespace fs = std::filesystem;

[[nodiscard]] inline fs::path user_tmp_dir() {
#if defined(_WIN32)
    auto const dir = fs::temp_directory_path() / "comdare_prtart_test";
#else
    auto const dir = fs::temp_directory_path() / ("comdare_prtart_test_" + std::to_string(::getuid()));
#endif
    std::error_code ec;
    fs::create_directories(dir, ec); // best effort; Fehler zeigt sich beim ersten Schreibzugriff
    return dir;
}

// ---------------------------------------------------------------------------
// (1) DER WUERFEL. /dev/urandom, nicht std::random_device und erst recht keine
// Konstante aus einer Doku: der Wert darf in KEINER Datei dieses Repos vorkommen.
// Gibt die Quelle nichts her, ist das ein ROTER Fall und kein stiller Ersatzwert --
// ohne frischen Koeder gibt es keinen Beweis.
// ---------------------------------------------------------------------------
[[nodiscard]] inline std::string koeder(std::size_t bytes = 6) {
    std::ifstream quelle{"/dev/urandom", std::ios::binary};
    EXPECT_TRUE(quelle.good()) << "/dev/urandom nicht lesbar -- ohne frischen Koeder kein Beweis";
    std::vector<unsigned char> roh(bytes, 0U);
    quelle.read(reinterpret_cast<char*>(roh.data()), static_cast<std::streamsize>(bytes));
    EXPECT_EQ(quelle.gcount(), static_cast<std::streamsize>(bytes))
        << "/dev/urandom lieferte zu wenige Bytes -- fail-closed statt Ersatzwert";
    static constexpr char kZiffern[] = "0123456789abcdef";
    std::string           marke;
    for (unsigned char const b : roh) {
        marke.push_back(kZiffern[(b >> 4U) & 0x0FU]);
        marke.push_back(kZiffern[b & 0x0FU]);
    }
    return marke;
}

// ---------------------------------------------------------------------------
// (2) DER WEGWERF-GEGENSTAND: ein eigener Dateibaum unter einem eindeutigen
// Temp-Pfad. Der Bestand wird NIE angefasst: jeder Fall arbeitet unter seinem
// eigenen Pfad und raeumt ihn wieder ab.
// ---------------------------------------------------------------------------
class WegwerfRepo {
public:
    explicit WegwerfRepo(std::string const& marke) : wurzel_{user_tmp_dir() / ("wachen_werkbank_" + marke)} {
        std::error_code ec;
        fs::remove_all(wurzel_, ec);
        fs::create_directories(wurzel_, ec);
    }

    WegwerfRepo(WegwerfRepo const&)            = delete;
    WegwerfRepo& operator=(WegwerfRepo const&) = delete;
    WegwerfRepo(WegwerfRepo&&)                 = delete;
    WegwerfRepo& operator=(WegwerfRepo&&)      = delete;

    ~WegwerfRepo() {
        std::error_code ec;
        fs::remove_all(wurzel_, ec);
    }

    [[nodiscard]] fs::path const& pfad() const { return wurzel_; }

    [[nodiscard]] testing::AssertionResult schreibe(std::string const& relativ, std::string const& inhalt) const {
        fs::path const  ziel = wurzel_ / relativ;
        std::error_code ec;
        fs::create_directories(ziel.parent_path(), ec);
        std::ofstream aus{ziel, std::ios::binary | std::ios::trunc};
        if (!aus.good()) { return testing::AssertionFailure() << "konnte '" << ziel.string() << "' nicht anlegen"; }
        aus << inhalt;
        aus.close();
        if (!fs::exists(ziel)) {
            return testing::AssertionFailure() << "'" << ziel.string() << "' fehlt nach dem Schreiben";
        }
        return testing::AssertionSuccess();
    }

private:
    fs::path wurzel_;
};

} // namespace comdare::prt_art::test::wachen
