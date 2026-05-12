#pragma once
// PrefetchDistanceEstimator — Density-basierte Prefetch-Distanz fuer PRT-ART
// REV 6 §5.17
//
// Distanz = wie viele Cache-Lines im Voraus geprefetched werden.
// Bei dichten Knoten (Array<256>) → kurzer Lookup, kleine Distanz.
// Bei spaerlichen Knoten (Vector<u16,u16>) → grosser Lookup, grosse Distanz.

#include <cstddef>
#include <cstdint>

namespace comdare::prt_art::prefetch {

class PrefetchDistanceEstimator {
public:
    static constexpr std::uint8_t kMinDistance = 1;
    static constexpr std::uint8_t kMaxDistance = 16;

    // Schaetzt Prefetch-Distance abhaengig von Density (in Prozent) + Cache-Hierarchy-Tier-Latenz
    [[nodiscard]] static constexpr std::uint8_t estimate(double density_percent,
                                                         double tier_latency_cycles) noexcept {
        // Spaerliche Knoten profitieren von mehr Prefetch-Distance
        double sparseness = 1.0 - (density_percent / 100.0);
        // Latenz-Faktor: hoeher Latenz → mehr Distanz noetig (cache-line-coverage wichtiger)
        double latency_factor = tier_latency_cycles / 10.0;     // ca. 1 bei L1, 30 bei DRAM
        double estimate       = 1.0 + sparseness * 8.0 + latency_factor;
        if (estimate < kMinDistance) estimate = kMinDistance;
        if (estimate > kMaxDistance) estimate = kMaxDistance;
        return static_cast<std::uint8_t>(estimate);
    }

    [[nodiscard]] static constexpr std::uint8_t clamp(int raw) noexcept {
        if (raw < kMinDistance) return kMinDistance;
        if (raw > kMaxDistance) return kMaxDistance;
        return static_cast<std::uint8_t>(raw);
    }
};

}  // namespace comdare::prt_art::prefetch
