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

# ─────────────────────────────────────────────────────────────────────────────
# WP-4/F29 (Voll-Audit 2026-07-16) — Registry-Round-Trip-Gate (Muster R-E/M-CE-27, analog ce
# test_axis_registry_roundtrip): das VOR-gebaute Tool regeneriert die XML nach CMAKE_CURRENT_BINARY_DIR,
# registry_roundtrip.cmake byte-difft gegen die committete prt_art/algorithm_profiles/prt_art_axis_registry.xml
# (NUR GELESEN). Registrierung GEGATED auf COMDARE_CE_GENERATED_DIR: ohne konfigurierten ce-Geschwister-Build
# ist das Tool nicht kompilierbar (s.o.) — der Default-Standalone-ctest-Lauf bleibt unveraendert (kein
# Phantom-FAIL), das Gate laeuft ueberall dort, wo der Generator real baubar ist (CI-Heimat: der dedizierte
# registry-roundtrip-Job der .gitlab-ci.yml, der die Geschwister-ce zuvor konfiguriert; 2-Pass wie ce).
# Referenz-Konfiguration: s. registry_roundtrip.cmake-Kopf.
# ─────────────────────────────────────────────────────────────────────────────
if(COMDARE_CE_GENERATED_DIR AND COMDARE_PRT_ART_BUILD_TESTS)
    add_test(
        NAME test_prt_registry_roundtrip
        COMMAND "${CMAKE_COMMAND}"
            "-DGEN_TOOL=$<TARGET_FILE:prt_art_axis_registry_gen>"
            "-DCOMMITTED=${CMAKE_CURRENT_SOURCE_DIR}/../algorithm_profiles/prt_art_axis_registry.xml"
            "-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/registry_roundtrip_prt"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/registry_roundtrip.cmake")
    set_tests_properties(test_prt_registry_roundtrip
        PROPERTIES LABELS "registry;roundtrip" TIMEOUT 120)
endif()
