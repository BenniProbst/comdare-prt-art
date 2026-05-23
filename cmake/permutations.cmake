# PRT-ART permutations.cmake — Pruefling-Algorithmus-Rekombinationen
# V36.C (2026-05-23) — analog cache-engine/cmake/permutations.cmake
#
# Achsen (Pruefling-spezifisch):
#   PA-1: Node-Typ (compact/wide/leaf_only/internal)
#   PA-2: Path-Compression (none/lazy/eager)
#   PA-3: Lookup-Algorithmus (linear/binary/simd)
#   PA-4: Telemetry-Mode (leaf_only/sampling/offline)  [Kuehn 11.X1]
#
# Tri-State Mode + Default on_build_on_demand (User-Direktive 2026-05-23)

set(_pa_root "${CMAKE_CURRENT_LIST_DIR}/..")
set(_pa_codegen_tools "${_pa_root}/prt_art/permutations_codegen")

set(COMDARE_PRT_ART_PERMUTATION_OUTPUT
    "${CMAKE_BINARY_DIR}/generated/prt_art_permutations.cmake")

set(COMDARE_PRT_ART_PERMUTATION_PROFILE "smoke" CACHE STRING
    "PRT-ART Profile: smoke (~16 Perms), medium (~81), full")
set_property(CACHE COMDARE_PRT_ART_PERMUTATION_PROFILE PROPERTY STRINGS smoke medium full)

set(COMDARE_PRT_ART_PERMUTATION_MODE "on_build_on_demand" CACHE STRING
    "PRT-ART Mode: on_rebuild | on_build_on_demand | off_pause_build")
set_property(CACHE COMDARE_PRT_ART_PERMUTATION_MODE PROPERTY STRINGS
    on_rebuild on_build_on_demand off_pause_build)

if(COMDARE_PRT_ART_PERMUTATION_MODE STREQUAL "off_pause_build")
    message(STATUS "PRT-ART Permutations: Build pausiert (off_pause_build).")
    return()
endif()

get_filename_component(_pa_out_dir "${COMDARE_PRT_ART_PERMUTATION_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${_pa_out_dir}")

set(_codegen_cmd
    ${CMAKE_COMMAND}
    "-DCOMDARE_PROFILE=${COMDARE_PRT_ART_PERMUTATION_PROFILE}"
    "-DCOMDARE_MODE=${COMDARE_PRT_ART_PERMUTATION_MODE}"
    "-DCOMDARE_OUTPUT=${COMDARE_PRT_ART_PERMUTATION_OUTPUT}"
    -P "${_pa_codegen_tools}/codegen.cmake")

message(STATUS "PRT-ART Permutations-Codegen (Profile=${COMDARE_PRT_ART_PERMUTATION_PROFILE}, Mode=${COMDARE_PRT_ART_PERMUTATION_MODE}) wird aufgerufen ...")
execute_process(
    COMMAND ${_codegen_cmd}
    WORKING_DIRECTORY "${_pa_root}"
    RESULT_VARIABLE _pa_codegen_result)

if(NOT _pa_codegen_result EQUAL 0)
    message(FATAL_ERROR "PRT-ART Permutations-Codegen fehlgeschlagen (exit ${_pa_codegen_result})")
endif()
