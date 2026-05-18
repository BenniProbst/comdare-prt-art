# V32.JJ.2 (2026-05-18 spaet) - prt-art Traversal Header-only Module

add_library(prt_art_traversal INTERFACE)
target_include_directories(prt_art_traversal INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
target_compile_features(prt_art_traversal INTERFACE cxx_std_23)

# Achse 3.A SearchAlgo-Traversal + Achse 3.M Traversal-Mapping (PRT-ART (b) Neu-Impl)
