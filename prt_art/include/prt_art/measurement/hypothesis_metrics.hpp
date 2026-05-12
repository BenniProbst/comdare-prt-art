#pragma once
// PRT-ART Hypothesis-Metriken (H1/H2/H3) — Diplomarbeit-Verifikations-Metriken
// REV 6 §5.17 + Habich-Anforderung
//
// H1 (Page-Type-Cost): mittlere Zugriffskosten pro Seitentyp
// H2 (Code-Qualitaet): pro Bausteine-Quelle ein Quality-Score
// H3 (Inline-vs-External): empirisch getroffene Verteilung der ValueHandle-Variant

#include <cstdint>
#include <string>
#include <unordered_map>

namespace comdare::prt_art::measurement {

struct H1PageTypeCost {
    std::unordered_map<std::string, double> mean_cycles_per_lookup_by_page_type;
    std::unordered_map<std::string, std::uint64_t> sample_count_by_page_type;

    void note_lookup(std::string const& page_type, double cycles) {
        auto& m = mean_cycles_per_lookup_by_page_type[page_type];
        auto& n = sample_count_by_page_type[page_type];
        // Online-Mean (Welford-Stil simplified)
        ++n;
        m += (cycles - m) / static_cast<double>(n);
    }
};

struct H2CodeQuality {
    std::unordered_map<std::string, double> quality_score_by_source;
    std::unordered_map<std::string, std::string> note_by_source;

    void score(std::string const& source, double s, std::string note = "") {
        quality_score_by_source[source] = s;
        if (!note.empty()) note_by_source[source] = std::move(note);
    }
};

struct H3InlineVsExternalDistribution {
    std::uint64_t inline_chosen   = 0;
    std::uint64_t external_chosen = 0;
    std::uint64_t chain_chosen    = 0;

    void note_inline()    noexcept { ++inline_chosen; }
    void note_external()  noexcept { ++external_chosen; }
    void note_chain()     noexcept { ++chain_chosen; }

    [[nodiscard]] std::uint64_t total() const noexcept {
        return inline_chosen + external_chosen + chain_chosen;
    }

    [[nodiscard]] double inline_share() const noexcept {
        auto t = total();
        return t > 0 ? static_cast<double>(inline_chosen) / static_cast<double>(t) : 0.0;
    }

    [[nodiscard]] double external_share() const noexcept {
        auto t = total();
        return t > 0 ? static_cast<double>(external_chosen) / static_cast<double>(t) : 0.0;
    }

    [[nodiscard]] double chain_share() const noexcept {
        auto t = total();
        return t > 0 ? static_cast<double>(chain_chosen) / static_cast<double>(t) : 0.0;
    }
};

struct PrtArtHypothesisMetrics {
    H1PageTypeCost                  h1{};
    H2CodeQuality                   h2{};
    H3InlineVsExternalDistribution  h3{};
};

}  // namespace comdare::prt_art::measurement
