#pragma once
// RedirectPrefetch — Special-Case fuer RedirectNode-Subtree-Pages
// REV 6 §5.17
//
// Wenn ein RedirectNode getroffen wird, ist mit hoher Wahrscheinlichkeit der
// terminale Knoten unmittelbar danach erreichbar — also lohnt sich ein Prefetch
// auf den vermuteten Endpunkt + die Cache-Lines davor/danach.

#include <cstdint>

namespace comdare::prt_art::prefetch {

class RedirectPrefetch {
public:
    static constexpr std::uint8_t kFanOut       = 3;     // 1 Hauptziel + 2 Geschwister
    static constexpr std::uint8_t kCacheLineSize = 64;

    void schedule(std::uint64_t terminal_addr) noexcept {
        last_scheduled_addr_ = terminal_addr;
        ++total_scheduled_;
    }

    [[nodiscard]] std::uint64_t last_scheduled_addr() const noexcept { return last_scheduled_addr_; }
    [[nodiscard]] std::uint64_t total_scheduled()    const noexcept { return total_scheduled_; }

    // Liefert den i-ten Prefetch-Slot fuer den letzten Schedule-Aufruf.
    // i in [0..kFanOut). Die Geschwister-Slots liegen +/- kCacheLineSize.
    [[nodiscard]] std::uint64_t slot(std::uint8_t i) const noexcept {
        switch (i) {
            case 0: return last_scheduled_addr_;
            case 1: return last_scheduled_addr_ + kCacheLineSize;
            case 2: return last_scheduled_addr_ >= kCacheLineSize
                           ? last_scheduled_addr_ - kCacheLineSize
                           : 0;
            default: return last_scheduled_addr_;
        }
    }

    void reset() noexcept {
        last_scheduled_addr_ = 0;
        total_scheduled_     = 0;
    }

private:
    std::uint64_t last_scheduled_addr_ = 0;
    std::uint64_t total_scheduled_     = 0;
};

}  // namespace comdare::prt_art::prefetch
