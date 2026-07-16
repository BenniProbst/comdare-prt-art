// INC-B (2026-07-14) - prt_art_axis_registry_gen: erzeugt die prt-art-Registry-XML
// `prt_art_axis_registry.xml` per COMPILE-TIME-Reflektion der prt-art-Slot-Wrapper.
//
// WAS: eigenstaendiges C++-Executable (analog ce/tools/axis_registry_gen INC-A, KEINE CMake-Generierungs-
// Logik). Es reflektiert je prt-art-Slot dessen `PrueflingVariants`-mp_list (genau EIN Wrapper je Slot) +
// das golden-verdrahtete ce-seitige Merge-Organ und emittiert je Baustein name()/FQ-Typ/Kurz-Typ/header/
// enabled/golden_wired. GLEICHES Schema wie die ce-Registry (`<comdare_axis_registry>` -> `<axis>` ->
// `<baustein>`), engine="prt_art".
//
// prt-art bleibt ein SEPARATER PRUEFLING: dieser Generator + die XML manifestieren NUR prt-arts eigenen
// Slot-Beitrag; er mergt prt-art NICHT in den CEB. Die 4 repo-Slots sind Slot-DEMONSTRATIONEN
// (golden_wired="false"); golden_wired="true" traegt EINZIG das PathCompression-Merge-Organ (der einzige
// in die SOTA-Merge-Serien verdrahtete prt-art-Beitrag).
//
// R-B (Fork, verifiziert 20260713-verify-registry-facts §2c): die 5 Wrapper trugen bis INC-B KEIN
// cpp_type_name/header_include. Das per-Organ-Location-Makro COMDARE_DEFINE_ORGAN_LOCATION
// (anatomy/organ_location.hpp) schliesst diese Luecke -> 'type'/'header' sind jetzt reflektierbar (nicht
// mehr leer wie in der ce-Registry INC-A). Reflektion liest ausschliesslich diese constexpr-Member; es wird
// kein Typ-Name aus dem Symbol abgeleitet.
//
// KOLLISION (verifiziert §2b): das golden-verdrahtete PathCompression-Organ ist die BARE ce-Strategie
// PatriciaPathCompression (name()="path_compression_patricia") - KEIN prtart_-Name. Es ist der EINE Wrapper
// von PrtArtPathCompressionSlot::PrueflingVariants (compositions/prt_art_merge_reference.hpp:42-43). Es wird
// hier DIREKT reflektiert (statt via merge_reference.hpp, das die 6 SOTA-Host-Kompositionen zoege) - das
// Ergebnis ist byte-identisch (die mp_list hat genau dieses eine Element), der Include bleibt schlank.
//
// C++23. Ausfuehren: `prt_art_axis_registry_gen --out <pfad>`.

#include <prt_art/slots/axis_01_page_type_slot.hpp>    // slots::axis_01::{PrtArtBPlusPageType, Slot}
#include <prt_art/slots/axis_07_prefetch_slot.hpp>     // slots::axis_07::{PrtArtRedirectPrefetch, Slot}
#include <prt_art/slots/axis_11_telemetry_slot.hpp>    // slots::axis_11::{PrtArtPerNodeCounter, Slot}
#include <prt_art/slots/axis_14_value_handle_slot.hpp> // slots::axis_14::{PrtArtChainRefHandle, Slot}

// Das golden-verdrahtete ce-seitige Merge-Organ (bare Patricia; der einzige Baustein von
// PrtArtPathCompressionSlot::PrueflingVariants).
#include <axes/path_compression/axis_02_path_compression_patricia.hpp>

#include <anatomy/anatomy_base.hpp>   // AnatomyGenus + genus_name (F30: genus reflektiert statt Literal)
#include <anatomy/organ_location.hpp> // HasOrganLocation (R-B)

#include <boost/mp11.hpp>

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace cea = ::comdare::cache_engine::anatomy;
namespace mp  = boost::mp11;

namespace {

struct Baustein {
    std::string name;    // Wrapper::name() - byte-genauer serialize-Schluessel
    std::string wrapper; // Kurz-Typ (letzte ::-Komponente vor einem eventuellen '<')
    std::string type;    // fully-qualified C++-Typ (Wrapper::cpp_type_name, R-B)
    std::string header;  // Include-Pfad (Wrapper::header_include, R-B)
    bool        enabled = true;
};

struct AxisOut {
    std::string           slot;     // axis_NN (Slot-Nummer der CE-Achse)
    std::string           id;       // page_type / prefetch / telemetry / value_handle / path_compression
    std::string           category; // pruefling_slot | golden_merge_slot
    std::string           genus;    // F30 (2026-07-16): aus Slot::genus reflektiert (genus_name), nicht Literal
    bool                  golden = false;
    std::vector<Baustein> bausteine;
};

// Kurz-Typ aus dem FQ-Namen: alles vor einem eventuellen '<', dann hinter dem letzten "::".
[[nodiscard]] std::string short_name(std::string const& fq) {
    std::size_t const lt   = fq.find('<');
    std::string const head = (lt == std::string::npos) ? fq : fq.substr(0, lt);
    std::size_t const cc   = head.rfind("::");
    return (cc == std::string::npos) ? head : head.substr(cc + 2);
}

[[nodiscard]] std::string xml_escape(std::string const& s) {
    std::string o;
    o.reserve(s.size());
    for (char const c : s) {
        switch (c) {
            case '&': o += "&amp;"; break;
            case '<': o += "&lt;"; break;
            case '>': o += "&gt;"; break;
            case '"': o += "&quot;"; break;
            case '\'': o += "&apos;"; break;
            default: o += c;
        }
    }
    return o;
}

// Reflektiert EINEN Wrapper (name/FQ-Typ/Kurz-Typ/header/enabled). R-B: Typ+Header aus dem
// per-Organ-Location-Makro; static_assert garantiert, dass der Wrapper das Makro traegt.
template <class W>
[[nodiscard]] Baustein reflect_wrapper() {
    static_assert(cea::HasOrganLocation<W>,
                  "R-B: Wrapper muss COMDARE_DEFINE_ORGAN_LOCATION tragen (cpp_type_name + header_include).");
    Baustein b;
    b.name    = std::string{W::name()};
    b.type    = std::string{W::cpp_type_name};
    b.wrapper = short_name(b.type);
    b.header  = std::string{W::header_include};
    b.enabled = W::enabled;
    return b;
}

// Reflektiert die (1-elementige) PrueflingVariants-mp_list eines prt-art-Slots. mp_identity vermeidet die
// Default-Konstruktion der Wrapper (nur der Typ wird benoetigt).
template <class Slot>
[[nodiscard]] std::vector<Baustein> reflect_slot() {
    std::vector<Baustein> out;
    mp::mp_for_each<mp::mp_transform<mp::mp_identity, typename Slot::PrueflingVariants>>([&](auto id) {
        using W = typename decltype(id)::type;
        out.push_back(reflect_wrapper<W>());
    });
    return out;
}

template <class Slot>
[[nodiscard]] AxisOut make_slot(char const* slot, char const* id, char const* category, bool golden) {
    // F30 (2026-07-16): genus aus dem Slot-Member reflektiert (Enum-Single-Source anatomy_base.hpp genus_name),
    // nicht mehr als String-Literal emittiert — byte-identisches Ergebnis ("SearchAlgorithm"), aber Enum-Drift
    // (neuer Genus am Slot) schluege jetzt ehrlich in der XML durch.
    return AxisOut{slot, id, category, std::string{cea::genus_name(Slot::genus)}, golden, reflect_slot<Slot>()};
}

// Fuer das golden-verdrahtete ce-seitige Merge-Organ: der Wrapper wird direkt reflektiert (er IST der
// einzige Baustein von PrtArtPathCompressionSlot::PrueflingVariants). genus: der Wrapper selbst traegt kein
// Slot-genus-Member; der zugehoerige PrtArtPathCompressionSlot lebt in merge_reference.hpp (dessen Include die
// Kopf-Doku bewusst vermeidet — er zoege die 6 SOTA-Host-Kompositionen). Daher hier direkt aus der Enum-
// Single-Source reflektiert (genus_name(SearchAlgorithm)) — derselbe Wert, kein freies String-Literal.
template <class W>
[[nodiscard]] AxisOut make_wrapper(char const* slot, char const* id, char const* category, bool golden) {
    AxisOut a{slot, id, category, std::string{cea::genus_name(cea::AnatomyGenus::SearchAlgorithm)}, golden, {}};
    a.bausteine.push_back(reflect_wrapper<W>());
    return a;
}

// GUARD: kein clean serialize-Schluessel enthaelt '(' oder ' '. Faengt jede konditionale
// (disabled)-Form + versehentliche Family-Namen-Leaks. Leerer name ebenfalls illegal.
[[nodiscard]] bool name_is_clean(std::string const& n) {
    if (n.empty()) return false;
    return n.find('(') == std::string::npos && n.find(' ') == std::string::npos;
}

} // namespace

int main(int argc, char** argv) {
    std::string out_path = "prt_art_axis_registry.xml";
    for (int i = 1; i < argc; ++i) {
        std::string_view const a = argv[i];
        if (a == "--out" && i + 1 < argc) {
            out_path = argv[++i];
        } else if (a == "--help" || a == "-h") {
            std::cout << "prt_art_axis_registry_gen --out <file>\n";
            return 0;
        } else {
            std::cerr << "prt_art_axis_registry_gen: unbekanntes Argument '" << a << "'\n";
            return 2;
        }
    }

    namespace s01 = ::comdare::prt_art::slots::axis_01;
    namespace s07 = ::comdare::prt_art::slots::axis_07;
    namespace s11 = ::comdare::prt_art::slots::axis_11;
    namespace s14 = ::comdare::prt_art::slots::axis_14;

    std::vector<AxisOut> axes;
    // -- Die 4 prt-art-repo-Slots (Slot-Demonstration; golden_wired stets false). --
    axes.push_back(make_slot<s01::Slot>("axis_01", "page_type", "pruefling_slot", false));
    axes.push_back(make_slot<s07::Slot>("axis_07", "prefetch", "pruefling_slot", false));
    axes.push_back(make_slot<s11::Slot>("axis_11", "telemetry", "pruefling_slot", false));
    axes.push_back(make_slot<s14::Slot>("axis_14", "value_handle", "pruefling_slot", false));
    // -- Das 1 golden-verdrahtete ce-seitige PathCompression-Merge-Organ (bare Patricia; §2b-Kollision). --
    axes.push_back(make_wrapper<::comdare::cache_engine::path_compression::PatriciaPathCompression>(
        "axis_02", "path_compression", "golden_merge_slot", true));

    // -- GUARD: kein name() darf '(' / ' ' / leer sein. --
    std::size_t total = 0;
    for (auto const& ax : axes) {
        for (auto const& b : ax.bausteine) {
            ++total;
            if (!name_is_clean(b.name)) {
                std::cerr << "prt_art_axis_registry_gen: GUARD-BRUCH - unsauberer name() '" << b.name << "' in Slot '"
                          << ax.id << "'. KEINE Datei geschrieben.\n";
                return 3;
            }
        }
    }

    std::ofstream f{out_path, std::ios::binary};
    if (!f) {
        std::cerr << "prt_art_axis_registry_gen: kann Ausgabedatei nicht oeffnen: " << out_path << "\n";
        return 4;
    }

    f << "<comdare_axis_registry engine=\"prt_art\" schema=\"1\">\n";
    f << "  <!-- GENERIERT von prt_art/registry_gen (INC-B) per compile-time-Reflektion der prt-art-Slot-\n";
    f << "       Wrapper (PrueflingVariants). NICHT von Hand editieren. Gleiches Schema wie die ce-Registry\n";
    f << "       (cache_engine_axis_registry.xml, engine=\"cache_engine\"); hier engine=\"prt_art\". -->\n";
    f << "  <!-- prt-art bleibt ein SEPARATER PRUEFLING: diese Registry manifestiert NUR prt-arts Slot-Beitrag,\n";
    f << "       sie mergt prt-art NICHT in den CEB. Die 4 repo-Slots = Slot-Demonstration (golden_wired=false);\n";
    f << "       golden_wired=true traegt EINZIG das PathCompression-Merge-Organ (SOTA-Merge-Serien). -->\n";
    f << "  <!-- 'header' ist per R-B (COMDARE_DEFINE_ORGAN_LOCATION) befuellt. KOLLISION: das golden-Organ ist\n";
    f << "       die bare ce-Strategie PatriciaPathCompression (name=path_compression_patricia, KEIN prtart_-\n";
    f << "       Name) - der ce-Patricia-Baustein, den die prt-art-SOTA-Merge in den path_compression-Slot setzt. "
         "-->\n";
    for (auto const& ax : axes) {
        f << "  <axis id=\"" << xml_escape(ax.id) << "\" slot=\"" << xml_escape(ax.slot) << "\" category=\""
          << xml_escape(ax.category) << "\" genus=\"" << xml_escape(ax.genus) << "\" baustein_count=\""
          << ax.bausteine.size() << "\">\n";
        for (auto const& b : ax.bausteine) {
            f << "    <baustein name=\"" << xml_escape(b.name) << "\" wrapper=\"" << xml_escape(b.wrapper)
              << "\" type=\"" << xml_escape(b.type) << "\" header=\"" << xml_escape(b.header) << "\" enabled=\""
              << (b.enabled ? "true" : "false") << "\" golden_wired=\"" << (ax.golden ? "true" : "false") << "\"/>\n";
        }
        f << "  </axis>\n";
    }
    f << "</comdare_axis_registry>\n";
    f.flush();
    if (!f) {
        std::cerr << "prt_art_axis_registry_gen: Schreibfehler an " << out_path << "\n";
        return 4;
    }

    std::cout << "prt_art_axis_registry_gen: " << axes.size() << " Slots, " << total << " Bausteine -> " << out_path
              << "\n";
    for (auto const& ax : axes) {
        std::cout << "  " << ax.slot << " " << ax.id << " : " << ax.bausteine.size()
                  << (ax.golden ? " (golden_wired)" : "") << "\n";
    }
    return 0;
}
