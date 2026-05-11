#pragma once
// BPlusNode (P0) — PRT-ART eigener kompakter B+-aehnlicher Knoten
// REV 6 §5.17 + REV 5 K05
//
// Zweck: Verzweigungs-Knoten mit cache-line-aligned Slot-Layout. Im Gegensatz zu
// klassischen B+-Knoten haelt BPlusNode keine Daten — nur 1-Byte-Diskriminatoren
// + Child-Indizes. Die eigentliche Such-Operation pro Slot wird einer der vier
// internal-search-Strukturen (Array<256>, Array<65535>, Vector<u8,u8>,
// Vector<u16,u16>) ueberlassen, abhaengig von der Density.
//
// Die Density-Schwellwerte (25/50/75%) entscheiden, welche internal-search
// genutzt wird (siehe internal_search/*).

#include <array>
#include <cstdint>
#include <vector>

namespace comdare::prt_art::nodes {

enum class InternalSearchKind : std::uint8_t {
    Array256       = 0,   // Density bis 25%
    Array65535     = 1,   // Density 25-50%
    VectorU8U8     = 2,   // Density 50-75%
    VectorU16U16   = 3,   // Density >75%
};

class BPlusNode {
public:
    static constexpr std::uint8_t kPriorityP0 = 0;

    explicit BPlusNode(InternalSearchKind kind = InternalSearchKind::Array256)
        : kind_(kind) {}

    [[nodiscard]] InternalSearchKind kind() const noexcept { return kind_; }

    void set_kind(InternalSearchKind k) noexcept { kind_ = k; }

    void insert_slot(std::uint8_t discriminator, std::uint64_t child_index) {
        slots_.push_back({discriminator, child_index});
        ++key_count_;
    }

    [[nodiscard]] std::size_t key_count() const noexcept { return key_count_; }

    // Density = key_count / kapazitaet[kind] (in %), kapazitaet abhaengig von Kind
    [[nodiscard]] static constexpr std::size_t capacity_for(InternalSearchKind k) noexcept {
        switch (k) {
            case InternalSearchKind::Array256:     return 256;
            case InternalSearchKind::Array65535:   return 65535;
            case InternalSearchKind::VectorU8U8:   return 256;        // Vector mit u8-Diskriminatoren
            case InternalSearchKind::VectorU16U16: return 65535;
        }
        return 256;
    }

    [[nodiscard]] double density_percent() const noexcept {
        std::size_t cap = capacity_for(kind_);
        if (cap == 0) return 0.0;
        return (static_cast<double>(key_count_) / static_cast<double>(cap)) * 100.0;
    }

    // Empfohlene Reklassifizierung anhand aktueller Density (25/50/75%-Schwellwerte)
    [[nodiscard]] InternalSearchKind recommended_kind() const noexcept {
        double d = density_percent();
        if (d >= 75.0) return InternalSearchKind::VectorU16U16;
        if (d >= 50.0) return InternalSearchKind::VectorU8U8;
        if (d >= 25.0) return InternalSearchKind::Array65535;
        return InternalSearchKind::Array256;
    }

    struct Slot {
        std::uint8_t  discriminator;
        std::uint64_t child_index;
    };

    [[nodiscard]] std::vector<Slot> const& slots() const noexcept { return slots_; }

private:
    InternalSearchKind kind_;
    std::vector<Slot>  slots_{};
    std::size_t        key_count_ = 0;
};

}  // namespace comdare::prt_art::nodes
