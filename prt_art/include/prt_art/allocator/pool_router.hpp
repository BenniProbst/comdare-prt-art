#pragma once
// PoolRouter — Routing einer Allocation-Anfrage zum passenden Pool
// REV 6 §5.17
//
// Pages werden nach ihrem Encoding (Redirect/DenseByte/MultilevelDense/SparsePatricia/
// DecisionSpan/CustomAligned) auf die 4 Page-Pools (A/B/C/D) abgebildet.
// Werte werden nach HandleKind (Inline → V-static, External/ChainRef → V-dynamic) verteilt.
// Restallokationen → R-Pool.

#include <prt_art/allocator/pool_descriptor.hpp>

#include <cstdint>

namespace comdare::prt_art::allocator {

enum class PageEncodingTag : std::uint8_t {
    Redirect        = 0, // → Pool A
    DenseByte       = 1, // → Pool B
    MultilevelDense = 2, // → Pool C
    SparsePatricia  = 3, // → Pool B (kompakte Pages)
    DecisionSpan    = 4, // → Pool D
    CustomAligned   = 5, // → Pool D (eigene Optimierung)
};

enum class ValueHandleTag : std::uint8_t {
    Inline   = 0, // → Pool V-static
    External = 1, // → Pool V-dynamic
    ChainRef = 2, // → Pool V-dynamic
};

class PoolRouter {
public:
    [[nodiscard]] static constexpr PoolKind route_page(PageEncodingTag enc) noexcept {
        switch (enc) {
            case PageEncodingTag::Redirect: return PoolKind::A_TrieHuelle;
            case PageEncodingTag::DenseByte: return PoolKind::B_DensePages;
            case PageEncodingTag::MultilevelDense: return PoolKind::C_MultiLevel;
            case PageEncodingTag::SparsePatricia: return PoolKind::B_DensePages;
            case PageEncodingTag::DecisionSpan: return PoolKind::D_DecisionSpan;
            case PageEncodingTag::CustomAligned: return PoolKind::D_DecisionSpan;
        }
        return PoolKind::R_Rest;
    }

    [[nodiscard]] static constexpr PoolKind route_value(ValueHandleTag tag) noexcept {
        switch (tag) {
            case ValueHandleTag::Inline: return PoolKind::V_StaticValue;
            case ValueHandleTag::External: return PoolKind::V_DynamicValue;
            case ValueHandleTag::ChainRef: return PoolKind::V_DynamicValue;
        }
        return PoolKind::R_Rest;
    }

    [[nodiscard]] static constexpr PoolKind route_misc() noexcept { return PoolKind::R_Rest; }
};

} // namespace comdare::prt_art::allocator
