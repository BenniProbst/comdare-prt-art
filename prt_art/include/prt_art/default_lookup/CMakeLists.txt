# V32.JJ.2 (2026-05-18 spaet) - prt-art Default-Lookup Header-only Module

add_library(prt_art_default_lookup INTERFACE)
target_include_directories(prt_art_default_lookup INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
target_compile_features(prt_art_default_lookup INTERFACE cxx_std_23)

# Default-Lookup-Indikatoren signalisieren via Doxygen: PRT-ART hat
# auf diesen Achsen KEINE eigene Impl. CEB-AutoPermutator nutzt CE-Bibliothek.
