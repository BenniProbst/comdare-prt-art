#pragma once
// ValueHandle — std::variant<Inline, External, ChainRef> Wrapper (REV 6 §5.4)
//
// Pflicht-Datenpfad: drei Auspraegungen werden Compile-Time und Runtime parametrisierbar.
// Visitor-Interface erlaubt den nutzenden Knoten den jeweiligen Handle-Typ zu lesen.

#include <prt_art/value_handle/chain_ref_handle.hpp>
#include <prt_art/value_handle/external_handle.hpp>
#include <prt_art/value_handle/inline_handle.hpp>

#include <cstdint>
#include <variant>

namespace comdare::prt_art::value_handle {

enum class HandleKind : std::uint8_t {
    Inline   = 0,
    External = 1,
    ChainRef = 2,
};

template <std::size_t InlineCapacity = 8>
class ValueHandle {
public:
    using Inline_t   = InlineHandle<InlineCapacity>;
    using External_t = ExternalHandle;
    using Chain_t    = ChainRefHandle;
    using Storage_t  = std::variant<Inline_t, External_t, Chain_t>;

    ValueHandle() = default;
    explicit ValueHandle(Inline_t v) : storage_(std::move(v)) {}
    explicit ValueHandle(External_t v) : storage_(std::move(v)) {}
    explicit ValueHandle(Chain_t v) : storage_(std::move(v)) {}

    [[nodiscard]] HandleKind kind() const noexcept { return static_cast<HandleKind>(storage_.index()); }

    [[nodiscard]] bool is_inline() const noexcept { return std::holds_alternative<Inline_t>(storage_); }
    [[nodiscard]] bool is_external() const noexcept { return std::holds_alternative<External_t>(storage_); }
    [[nodiscard]] bool is_chain_ref() const noexcept { return std::holds_alternative<Chain_t>(storage_); }

    [[nodiscard]] Inline_t const&   as_inline() const { return std::get<Inline_t>(storage_); }
    [[nodiscard]] External_t const& as_external() const { return std::get<External_t>(storage_); }
    [[nodiscard]] Chain_t const&    as_chain_ref() const { return std::get<Chain_t>(storage_); }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& v) const {
        return std::visit(std::forward<Visitor>(v), storage_);
    }

private:
    Storage_t storage_{Inline_t{}};
};

} // namespace comdare::prt_art::value_handle
