#pragma once
// V41.F.6.1 Phase B — prt-art axis_01 Page-Type Slot (Plugin-Controller Slot-Füllung).
//
// Zweite Slot-Füllung (nach axis_07): prt-art liefert einen CE-konformen PAGE-TYPE-Wrapper
// (PrtArtBPlusPageType) für die Achse-1 (6 Pflicht-Seitentypen). Trägt prt-arts
// density-getriebene internal-search-Auswahl (25/50/75%-Schwellen, REV 6 §5.17 — Diplomarbeit-
// Kern). Wird via pruefling_merge ZUR COMPILE-TIME in den CE-Permutationsraum gemergt.
//
// Algorithmus: prt-arts BPlusNode (cache-line-aligned Verzweigungsseite mit Density-Dispatch).

#include <topics/nodes/axis_01_page_type/axis_01_page_type_strategy_base.hpp>
#include <topics/nodes/axis_01_page_type/concepts/axis_01_page_type_concept.hpp>
#include <topics/nodes/axis_01_page_type/concepts/axis_01_page_type_cache_engine_permutation_concept.hpp>
#include <topics/nodes/concepts/topic_nodes_concept.hpp>
#include <anatomy/pruefling_merge.hpp>   // PrueflingSlot-Pattern + AnatomyGenus

#include <prt_art/nodes/bplus_node.hpp>  // prt-art-Algorithmus-Logik (Density-Dispatch)

#include <boost/mp11.hpp>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace comdare::prt_art::slots::axis_01 {

namespace ce01 = ::comdare::cache_engine::nodes::axis_01_page_type;
namespace pf   = ::comdare::cache_engine::anatomy::pruefling;
namespace mp   = ::boost::mp11;

/// prt-art-eigene Subaxis-Markierung (density-getriebene Verzweigungsseite).
struct density_branch_tag {};

/// PrtArtBPlusPageType — CE-konformer axis_01-PAGE-TYPE-Wrapper über prt-arts BPlusNode.
/// Seitentyp BPlus (Verzweigung), erweitert um prt-arts density-getriebene internal-search-
/// Auswahl. Erfüllt PageTypeStrategy + CacheEnginePermutationStrategy → via pruefling_merge
/// in den CE-Permutationsraum einsetzbar (Stufe 2/3).
class PrtArtBPlusPageType : public ce01::PageTypeStrategyBase<PrtArtBPlusPageType> {
public:
    using topic_tag = ::comdare::cache_engine::nodes::concepts::NodesTopicTag;
    using axis_tag  = density_branch_tag;
    using family_id = std::integral_constant<int, 16>;  // prt-art Page-Type-Range (distinkt zu CE 0-5)

    static constexpr bool enabled = true;  // Pruefling-Variante: aktiv wenn der Slot geladen ist

    [[nodiscard]] static constexpr ce01::concepts::PageKind page_kind() noexcept {
        return ce01::concepts::PageKind::BPlus;
    }
    [[nodiscard]] static constexpr bool             is_branch()   noexcept { return true; }
    [[nodiscard]] static constexpr bool             is_leaf()     noexcept { return false; }
    [[nodiscard]] static constexpr std::string_view name()        noexcept { return "prtart_bplus_page"; }
    [[nodiscard]] static constexpr std::string_view family_name() noexcept { return "PrtArtBPlusPageType (density-driven branch page, prt-art REV6 §5.17)"; }
    [[nodiscard]] static constexpr std::string_view flag_suffix() noexcept { return "PRTART_BPLUS"; }

    using InternalSearchKind = ::comdare::prt_art::nodes::InternalSearchKind;

    // prt-art-Algorithmus-Logik (BPlusNode density-dispatch) — forwarded.
    void insert_slot(std::uint8_t discriminator, std::uint64_t child_index) {
        node_.insert_slot(discriminator, child_index);
    }
    [[nodiscard]] std::size_t key_count()        const noexcept { return node_.key_count(); }
    [[nodiscard]] double      density_percent()  const noexcept { return node_.density_percent(); }
    [[nodiscard]] InternalSearchKind kind()      const noexcept { return node_.kind(); }
    void set_kind(InternalSearchKind k)                noexcept { node_.set_kind(k); }
    /// Diplomarbeit-Kern: density-getriebene Reklassifizierung (25/50/75%-Schwellen).
    [[nodiscard]] InternalSearchKind recommended_kind() const noexcept { return node_.recommended_kind(); }

private:
    ::comdare::prt_art::nodes::BPlusNode node_{};
};

/// Slot — pruefling_merge-konformer axis_01-Slot (SearchAlgorithm-Gattung).
struct Slot {
    using PrueflingVariants = mp::mp_list<PrtArtBPlusPageType>;
    static constexpr bool has_pruefling = true;
    static constexpr ::comdare::cache_engine::anatomy::AnatomyGenus genus =
        ::comdare::cache_engine::anatomy::AnatomyGenus::SearchAlgorithm;
};

static_assert(ce01::concepts::PageTypeStrategy<PrtArtBPlusPageType>);
static_assert(ce01::concepts::CacheEnginePermutationStrategy<PrtArtBPlusPageType>);
static_assert(pf::PrueflingSlotConcept<Slot>);
static_assert(pf::HasPruefling_v<Slot>);

}  // namespace comdare::prt_art::slots::axis_01
