# prt_art/legacy_reimpl — PRT-ART Pruefling-Re-Implementations (REV 7.6 V9.4)
#
# Verschoben aus comdare-cache-engine 2026-05-14 (V9.4).
# Default OFF — Aktivierung explizit fuer Beitritts-Verifikation noetig.

option(COMDARE_PRT_ART_BUILD_LEGACY_REIMPL
    "Build PRT-ART Pruefling-Re-Implementations (P11-P27)"
    OFF)

if(COMDARE_PRT_ART_BUILD_LEGACY_REIMPL)
    message(STATUS "[PRT-ART V9.4] Building legacy_reimpl/ (Pruefling-Beitritts-Verifikation).")
    foreach(dir
            P11-CSS-tree
            P12-CSB-tree
            P13-Hankins
            P14-Samuel
            P16-Bender-TreeLayout
            P17-Bender-CacheOblivious
            P18-Saikkonen-MultiLevel
            P19-Saikkonen-LayoutInvariant
            P21-Chen-PrefetchBPlus
            P22-Chen-Fractal
            P23-Khan-AdaptivePrefetch
            P24-NaderanTahan
            P26-Zhang-FGCS
            P27-Zhang-ASPLOS-Hierarchical)
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${dir}/CMakeLists.txt")
            add_subdirectory(${dir})
        endif()
    endforeach()
else()
    message(STATUS "[PRT-ART V9.4] legacy_reimpl/ skipped (default OFF).")
endif()
