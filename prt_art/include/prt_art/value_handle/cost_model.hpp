#pragma once
// PRT-ART ValueHandle Cost-Model (REV 6 §5.4 H3-Hypothese)
//
// Inline-vs-External-Grenze ist architektur+workloadabhaengig und MUSS aktiv ueber
// Kostenmodell gewaehlt werden. Diese Heuristik approximiert die Kosten:
//
//  cost_inline   = value_bytes_per_record + access_count * cache_line_pollution
//  cost_external = pointer_bytes + access_count * (pointer_chase_latency + extra_cacheline_load)
//
// Wenn cost_external < cost_inline → ExternalHandle bevorzugt.

#include <cstddef>
#include <cstdint>

namespace comdare::prt_art::value_handle {

struct WorkloadProfile {
    std::uint64_t access_count_per_value      = 1;     // wie oft wird der Wert gelesen?
    double        cache_line_bytes            = 64.0;
    double        pointer_chase_cycles        = 12.0;  // typisch L2-L3-Latenz pro Indirection
    double        cache_line_pollution_factor = 0.10;  // 10% Cache-Line-Verschmutzung pro Wert
    std::size_t   inline_capacity_bytes       = 8;     // entspricht InlineHandle<8>::kInlineCapacity
};

[[nodiscard]] constexpr bool inline_handle_fits(std::size_t value_bytes,
                                                WorkloadProfile const& w) noexcept {
    return value_bytes <= w.inline_capacity_bytes;
}

[[nodiscard]] constexpr double estimate_inline_cost(std::size_t value_bytes,
                                                    WorkloadProfile const& w) noexcept {
    if (value_bytes > w.inline_capacity_bytes) return 1e9;   // unmoeglich
    double base = static_cast<double>(value_bytes);
    double pollution = static_cast<double>(w.access_count_per_value) *
                       w.cache_line_pollution_factor *
                       (static_cast<double>(value_bytes) / w.cache_line_bytes);
    return base + pollution;
}

[[nodiscard]] constexpr double estimate_external_cost(std::size_t value_bytes,
                                                      WorkloadProfile const& w) noexcept {
    constexpr double pointer_bytes = 8.0;
    double extra_load_per_access = w.pointer_chase_cycles +
                                   (static_cast<double>(value_bytes) / w.cache_line_bytes);
    return pointer_bytes + static_cast<double>(w.access_count_per_value) * extra_load_per_access;
}

enum class HandleRecommendation : std::uint8_t {
    Inline   = 0,
    External = 1,
    ChainRef = 2,   // bei known multi-value
};

[[nodiscard]] constexpr HandleRecommendation recommend_handle(std::size_t value_bytes,
                                                              bool is_multi_value,
                                                              WorkloadProfile const& w) noexcept {
    if (is_multi_value) return HandleRecommendation::ChainRef;
    if (!inline_handle_fits(value_bytes, w)) return HandleRecommendation::External;
    double c_in = estimate_inline_cost(value_bytes, w);
    double c_ex = estimate_external_cost(value_bytes, w);
    return c_in <= c_ex ? HandleRecommendation::Inline : HandleRecommendation::External;
}

}  // namespace comdare::prt_art::value_handle
