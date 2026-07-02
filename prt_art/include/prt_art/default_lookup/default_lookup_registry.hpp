#pragma once
// V33.B.1 (2026-05-21) - DefaultLookupRegistry: Achsen-Liste fuer CE-Auto-Permutation
//
// @subsystem PA (Pruefling-Identifikation)
// @phase_owner PA
//
// Liefert die Liste der PRT-ART-Achsen die der CEB-AutoPermutator via
// CE-Bibliothek aufloesen soll (AA.3 Direktive).
//
// Stand AA.4 §0.3: PRT-ART definiert eigene Implementierungen fuer Achsen
// 1, 2, 3.A, 3.M, 4, 5, 6.1, 6.5, 7, 8.1, 10, 14. Fuer alle anderen
// Achsen (3.B, 6.2, 6.3, 6.4, 8.2, 9, 11, 12.x, 13.x) hat es kein
// eigenes Modul -> default-lookup in CE.

#include <array>
#include <string_view>

namespace comdare::prt_art::default_lookup {

/**
 * @brief DefaultLookupAxis - Beschreibung einer default-lookup-Achse
 * @subsystem PA
 */
struct DefaultLookupAxis {
    std::string_view axis_id;          ///< z.B. "11" oder "3.B"
    std::string_view human_name;       ///< z.B. "TELEMETRY-COLLECTION"
    std::string_view ce_library_path;  ///< CE-Pfad fuer Lookup
    std::string_view indicator_header; ///< Platzhalter-File in default_lookup/
};

/**
 * @brief DefaultLookupRegistry - statische Liste aller PRT-ART default-lookup-Achsen
 * @subsystem PA
 *
 * Synchron gehalten mit den prt_art_*_default.hpp Platzhalter-Files.
 * Wird vom CEB-AutoPermutator iteriert um pro Achse die CE-Bibliothek-Permutation
 * anzustossen (anstatt PRT-ART-eigene Implementierung).
 */
class DefaultLookupRegistry {
public:
    [[nodiscard]] static constexpr auto enumerate() {
        return std::array<DefaultLookupAxis, 9>{
            {{"3.B", "Cache-Memory-Traversal", "cache-engine/concepts/disciplines/array_discipline.hpp",
              "prt_art_3b_cache_traversal_default.hpp"},
             {"6.2", "Reclamation-Policy", "cache-engine/reclamation/rcu_reclaim/",
              "prt_art_62_reclamation_default.hpp"},
             {"6.3", "NUMA-Affinity", "cache-engine/concepts/numa_affinity.hpp", "prt_art_63_numa_default.hpp"},
             {"6.4", "Huge-Page-Policy", "cache-engine/allocators/portable_aligned_alloc.hpp",
              "prt_art_64_huge_page_default.hpp"},
             {"8.2", "Locking-Mode", "cache-engine/concepts/locking_mode.hpp", "prt_art_82_locking_default.hpp"},
             {"9", "ISA-Targeting", "cache-engine/IPlatformProbe Output", "prt_art_9_isa_default.hpp"},
             {"11", "Telemetry-Collection", "cache-engine/concepts/telemetry/", "prt_art_11_telemetry_default.hpp"},
             {"12", "Hardware-Strategy", "cache-engine/concepts/hardware_strategy.hpp",
              "prt_art_12_hardware_default.hpp"},
             {"13", "Scheduling-Strategy", "cache-engine/concepts/scheduling_strategy.hpp",
              "prt_art_13_scheduling_default.hpp"}}};
    }

    [[nodiscard]] static constexpr bool is_default_lookup(std::string_view axis_id) {
        for (const auto& axis : enumerate()) {
            if (axis.axis_id == axis_id) return true;
        }
        return false;
    }

    [[nodiscard]] static constexpr std::size_t count() { return enumerate().size(); }
};

} // namespace comdare::prt_art::default_lookup
