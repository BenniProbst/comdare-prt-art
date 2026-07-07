#pragma once
// ═══ QUARANTAENE (AP-2-neu/#236 W4, 2026-07-07) ═══ ALT-PFAD — NICHT Teil des Mess-Pfads!
// Der EINZIGE PRT-ART-Mess-Pfad ist der cache-engine-KATALOG-PFAD (sota_catalog -> echte
// PrtArtComposition als DLL; ce-Gegenbeweis: test_ap2_katalog_pfad_stubfrei). Diese Datei
// gehoert zu einem der drei Alt-Pfade (V18-Template / EE-A-B-Vergleich / Registry-run) und
// traegt bewusst Stub-/Surrogat-Code — sie darf NIE in den Mess-Pfad verdrahtet werden.
// V41.E11 Phase B — prt-art als cache-engine-Plugin (Pruefling-Factory).
//
// Implementiert die cache-engine Plugin-Controller-Schnittstelle
// (comdare::cache_engine::api::IPruefling / IPrueflingFactory). Die cache-engine laedt prt-art
// per COMDARE_CE_PRUEFLINGE (Doku 20), registriert diese Factory + erzeugt pro Achsen-Kombination
// einen Pruefling, den das Mess-Projekt (Diplomarbeit) gegen die CE-Achsen mergt + misst.
//
// HINWEIS (E6/#22 gated): run() nutzt vorerst einen self-contained Platzhalter-Workload, NICHT
// die echte PrtArtExecutionEngineAdapter — deren Header ziehen prt-art's nested (altes)
// cache-engine. Erst nach E6 (nested-cache-engine-Cleanup) wird der echte prt-art-Engine-Bridge
// eingehaengt (TODO unten). Dieser Header haengt bewusst NUR an cache-engine/api + std.

#include <cache_engine/api/i_pruefling_factory.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace comdare::prt_art::pruefling {

namespace api = ::comdare::cache_engine::api;

/// Ein prt-art-Pruefling fuer eine konkrete Achsen-Kombination.
class PrtArtPruefling final : public api::IPruefling {
public:
    explicit PrtArtPruefling(std::string axes) : axes_(std::move(axes)) {}

    [[nodiscard]] std::string_view name() const override { return "prt-art"; }
    [[nodiscard]] std::string_view version() const override { return "0.1.0"; }
    [[nodiscard]] std::string_view axes_signature() const override { return axes_; }

    // TODO(E6): echten PrtArtExecutionEngineAdapter einhaengen (nach nested-cache-engine-Cleanup).
    // Vorerst self-contained representativer Workload (insert+lookup), um den Plugin-Controller-
    // Mechanismus messbar zu machen.
    [[nodiscard]] int run(std::size_t n_ops, double& out_micros_per_op) override {
        if (n_ops == 0) {
            out_micros_per_op = 0.0;
            return 0;
        }
        auto                                             t0 = std::chrono::steady_clock::now();
        std::unordered_map<std::uint64_t, std::uint64_t> store;
        store.reserve(n_ops);
        for (std::size_t i = 0; i < n_ops; ++i) {
            std::uint64_t k = (i * 2654435761u) & 0xFFFFFF; // Knuth-Multiplikativ-Streuung
            store[k]        = i;
        }
        std::uint64_t sink = 0;
        for (std::size_t i = 0; i < n_ops; ++i) {
            std::uint64_t k  = (i * 2654435761u) & 0xFFFFFF;
            auto          it = store.find(k);
            if (it != store.end()) sink += it->second;
        }
        auto   t1         = std::chrono::steady_clock::now();
        double total_us   = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / 1000.0;
        out_micros_per_op = total_us / static_cast<double>(2 * n_ops);
        return (sink == ~0ull) ? 1 : 0; // sink-Nutzung verhindert Wegoptimierung; immer 0
    }

private:
    std::string axes_;
};

/// Factory: erzeugt prt-art-Prueflinge pro Achsen-Kombination.
class PrtArtPrueflingFactory final : public api::IPrueflingFactory {
public:
    [[nodiscard]] std::string_view pruefling_name() const override { return "prt-art"; }

    // Repraesentative prt-art-Achsen-Kombinationen (REV6 §5.17: Redirect+B+ Node-Typen,
    // 4 Such-Typen, Inline/External/ChainRef ValueHandle). Erweiterbar via optional_prt_art_impl.
    [[nodiscard]] std::vector<std::string_view> available_axes_combinations() const override {
        return {
            "page=redirect|node=bplus|vh=inline",
            "page=redirect|node=bplus|vh=chain_ref",
            "page=dense_byte|node=node256|vh=external_pool",
        };
    }

    [[nodiscard]] std::unique_ptr<api::IPruefling> create(std::string_view axes) override {
        return std::make_unique<PrtArtPruefling>(std::string(axes));
    }
};

/// Registriert die prt-art-Factory bei der cache-engine-Registry (vom Controller aufgerufen).
inline void register_prt_art_pruefling(api::IPrueflingRegistry& registry) {
    registry.register_factory(std::make_unique<PrtArtPrueflingFactory>());
}

} // namespace comdare::prt_art::pruefling
