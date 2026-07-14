# INC-B (2026-07-14) - prt_art_axis_registry_gen: eigenstaendiges C++-Tool, erzeugt
# prt_art_axis_registry.xml per compile-time-Reflektion der prt-art-Slot-Wrapper (analog
# ce/tools/axis_registry_gen). NUR ein Build-Target; die XML-Generierung ist ein Laufzeit-Schritt.
# EXCLUDE_FROM_ALL: der Standard-Build baut das Tool nicht mit; es wird explizit per Target gebaut.
#
# Braucht zusaetzlich zu comdare::prt_art_core (prt_art/include + cache-engine-Header):
#   (1) Boost.MP11 (header-only, in der cache-engine vendored),
#   (2) das generated/-Verzeichnis eines KONFIGURIERTEN cache-engine-Builds - das golden-verdrahtete
#       PathCompression-Merge-Organ (PatriciaPathCompression) zieht <axes/.../axis_02_path_compression_flags.hpp>
#       (CMake-generiert via configure_file). Pfad per -DCOMDARE_CE_GENERATED_DIR=<ce-build>/generated.

add_executable(prt_art_axis_registry_gen EXCLUDE_FROM_ALL main.cpp)
target_compile_features(prt_art_axis_registry_gen PRIVATE cxx_std_23)
target_link_libraries(prt_art_axis_registry_gen PRIVATE comdare::prt_art_core)

# (1) Boost.MP11 - in der cache-engine vendored (header-only).
set(_prtargen_mp11_inc "${COMDARE_CACHE_ENGINE_DIR}/cmake/third_party/boost_mp11/include")
if(EXISTS "${_prtargen_mp11_inc}/boost/mp11.hpp")
    target_include_directories(prt_art_axis_registry_gen PRIVATE "${_prtargen_mp11_inc}")
endif()

# (2) generated/ eines konfigurierten cache-engine-Builds (die CMake-generierten *_flags.hpp).
set(COMDARE_CE_GENERATED_DIR "" CACHE PATH
    "Pfad zum generated/-Verzeichnis eines konfigurierten cache-engine-Builds (fuer <axes/.../*_flags.hpp>)")
if(COMDARE_CE_GENERATED_DIR)
    target_include_directories(prt_art_axis_registry_gen PRIVATE "${COMDARE_CE_GENERATED_DIR}")
else()
    message(WARNING
        "prt_art_axis_registry_gen: COMDARE_CE_GENERATED_DIR nicht gesetzt.\n"
        "Das golden-verdrahtete PathCompression-Organ zieht die CMake-generierte flags.hpp der cache-engine.\n"
        "Setze -DCOMDARE_CE_GENERATED_DIR=<cache-engine-build>/generated (Build zuvor konfigurieren).")
endif()
