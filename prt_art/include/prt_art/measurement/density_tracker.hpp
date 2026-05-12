#pragma once
// DensityTracker — Tracking der Knoten-Densities ueber alle Knoten der PRT-ART
// REV 6 §5.17 — speist die Reklassifizierungs-Entscheidung in BPlusNode.recommended_kind()

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace comdare::prt_art::measurement {

struct DensityBucket {
    std::uint64_t node_count           = 0;
    double        sum_density_percent  = 0.0;

    [[nodiscard]] double avg_density_percent() const noexcept {
        return node_count > 0 ? (sum_density_percent / static_cast<double>(node_count)) : 0.0;
    }
};

class DensityTracker {
public:
    void record(std::uint64_t node_id, double density_percent) noexcept {
        per_node_[node_id] = density_percent;
        ++total_observations_;
    }

    [[nodiscard]] double density_for(std::uint64_t node_id) const noexcept {
        auto it = per_node_.find(node_id);
        return it == per_node_.end() ? 0.0 : it->second;
    }

    [[nodiscard]] std::uint64_t total_observations() const noexcept { return total_observations_; }
    [[nodiscard]] std::size_t   tracked_node_count() const noexcept { return per_node_.size(); }

    // Histogram: 4 Buckets analog zu den 4 internal-search-Density-Bereichen 25/50/75
    [[nodiscard]] std::vector<DensityBucket> histogram() const {
        std::vector<DensityBucket> h(4);
        for (auto const& [node, d] : per_node_) {
            std::size_t bucket;
            if      (d < 25.0) bucket = 0;
            else if (d < 50.0) bucket = 1;
            else if (d < 75.0) bucket = 2;
            else               bucket = 3;
            ++h[bucket].node_count;
            h[bucket].sum_density_percent += d;
        }
        return h;
    }

    void reset() noexcept {
        per_node_.clear();
        total_observations_ = 0;
    }

private:
    std::unordered_map<std::uint64_t, double> per_node_{};
    std::uint64_t                              total_observations_ = 0;
};

}  // namespace comdare::prt_art::measurement
