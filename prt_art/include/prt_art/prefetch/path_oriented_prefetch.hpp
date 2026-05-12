#pragma once
// PathOrientedPrefetch — folgt aktivem Suchpfad und prefetched die naechsten N Knoten
// REV 6 §5.17 — kombiniert mit PrefetchDistanceEstimator

#include <prt_art/prefetch/distance_estimator.hpp>

#include <cstdint>
#include <vector>

namespace comdare::prt_art::prefetch {

class PathOrientedPrefetch {
public:
    PathOrientedPrefetch() = default;

    // Aktualisiert die internen Prefetch-Slots mit der naechsten erwarteten Adresse.
    void enqueue(std::uint64_t addr) {
        recent_path_.push_back(addr);
        if (recent_path_.size() > kMaxTrackedSlots) {
            recent_path_.erase(recent_path_.begin(),
                               recent_path_.begin() + (recent_path_.size() - kMaxTrackedSlots));
        }
        ++total_enqueued_;
    }

    [[nodiscard]] std::uint64_t total_enqueued() const noexcept { return total_enqueued_; }
    [[nodiscard]] std::size_t   queue_depth()    const noexcept { return recent_path_.size(); }

    [[nodiscard]] std::vector<std::uint64_t> const& path() const noexcept { return recent_path_; }

    // Empfehlung fuer naechste prefetch-Adresse (basierend auf aktueller Trajektorie)
    [[nodiscard]] std::uint64_t suggest_next() const noexcept {
        if (recent_path_.size() < 2) return recent_path_.empty() ? 0 : recent_path_.back();
        // einfache Extrapolation: letzter Schritt extrapolieren
        std::uint64_t a = recent_path_[recent_path_.size() - 2];
        std::uint64_t b = recent_path_.back();
        std::uint64_t step = (b > a) ? (b - a) : 0;
        return b + step;
    }

    void reset() noexcept {
        recent_path_.clear();
        total_enqueued_ = 0;
    }

private:
    static constexpr std::size_t kMaxTrackedSlots = 16;
    std::vector<std::uint64_t>   recent_path_{};
    std::uint64_t                total_enqueued_ = 0;
};

}  // namespace comdare::prt_art::prefetch
