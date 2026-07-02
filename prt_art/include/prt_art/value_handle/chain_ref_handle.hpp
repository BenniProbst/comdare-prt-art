#pragma once
// ChainRefHandle (PRT-ART Pflicht-Datenpfad) — verkettete Referenz fuer Multi-Value
// REV 6 §5.4 — vertagt P3, aber Skelett vorhanden
//
// Bei Multi-Value-Schluesseln (mehrere Werte mit gleichem Key) verweist ChainRefHandle
// auf einen Linked-List-Head im Pool. Jeder Chain-Knoten hat (value_offset, next_offset).

#include <cstdint>

namespace comdare::prt_art::value_handle {

class ChainRefHandle {
public:
    static constexpr std::uint64_t kEmptyChain = static_cast<std::uint64_t>(-1);

    ChainRefHandle() = default;

    explicit ChainRefHandle(std::uint64_t chain_head_offset, std::uint32_t chain_length)
        : chain_head_offset_(chain_head_offset), chain_length_(chain_length) {}

    [[nodiscard]] std::uint64_t chain_head_offset() const noexcept { return chain_head_offset_; }
    [[nodiscard]] std::uint32_t chain_length() const noexcept { return chain_length_; }

    [[nodiscard]] bool is_empty() const noexcept { return chain_head_offset_ == kEmptyChain || chain_length_ == 0; }

    void prepend_node(std::uint64_t new_head_offset) noexcept {
        chain_head_offset_ = new_head_offset;
        ++chain_length_;
    }

    void clear() noexcept {
        chain_head_offset_ = kEmptyChain;
        chain_length_      = 0;
    }

private:
    std::uint64_t chain_head_offset_ = kEmptyChain;
    std::uint32_t chain_length_      = 0;
};

} // namespace comdare::prt_art::value_handle
