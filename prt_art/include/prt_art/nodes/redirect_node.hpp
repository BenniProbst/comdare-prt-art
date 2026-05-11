#pragma once
// RedirectNode (P0) — PRT-ART Node-Typ inspiriert von CoCo-Trie Redirect-Knoten
// REV 6 §5.17 + REV 5 K05 §5.2-§5.3
//
// Zweck: kollabierter eindeutiger Restpfad — speichert nur das Suffix bis zum
// terminalen Wert oder zum naechsten echten Verzweigungs-Knoten.
// Reduziert Trie-Hoehe drastisch bei spaerlichen Schluessel-Verteilungen.

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace comdare::prt_art::nodes {

class RedirectNode {
public:
    static constexpr std::uint8_t kPriorityP0 = 0;

    explicit RedirectNode(std::span<std::byte const> rest_suffix,
                          std::uint64_t terminal_handle = 0)
        : rest_suffix_(rest_suffix.begin(), rest_suffix.end()),
          terminal_handle_(terminal_handle) {}

    [[nodiscard]] std::span<std::byte const> rest_suffix() const noexcept {
        return rest_suffix_;
    }

    [[nodiscard]] std::uint64_t terminal_handle() const noexcept {
        return terminal_handle_;
    }

    [[nodiscard]] std::size_t bytes_consumed() const noexcept {
        return rest_suffix_.size();
    }

    // Match: vergleicht Rest-Suffix mit dem konsumierbaren Praefix des Suchschluessels.
    // Liefert Anzahl matched Bytes (= rest_suffix_.size() bei vollem Match).
    [[nodiscard]] std::size_t try_match(std::span<std::byte const> remaining_key) const noexcept {
        std::size_t n = std::min(rest_suffix_.size(), remaining_key.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (rest_suffix_[i] != remaining_key[i]) return i;
        }
        return n;
    }

    [[nodiscard]] bool is_complete_match(std::span<std::byte const> remaining_key) const noexcept {
        return remaining_key.size() == rest_suffix_.size() &&
               try_match(remaining_key) == rest_suffix_.size();
    }

private:
    std::vector<std::byte> rest_suffix_;
    std::uint64_t          terminal_handle_;
};

}  // namespace comdare::prt_art::nodes
