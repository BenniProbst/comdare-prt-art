# PRT-ART codegen.cmake — Pruefling-Algorithmus-Rekombinationen
# V36.C (2026-05-23) — analog cache-engine codegen.cmake, eigene Achsen.

cmake_minimum_required(VERSION 3.28)

if(NOT DEFINED COMDARE_PROFILE)
    set(COMDARE_PROFILE "smoke")
endif()
if(NOT DEFINED COMDARE_MODE)
    set(COMDARE_MODE "on_build_on_demand")
endif()
if(NOT DEFINED COMDARE_OUTPUT)
    message(FATAL_ERROR "COMDARE_OUTPUT erforderlich")
endif()

get_filename_component(_out_dir "${COMDARE_OUTPUT}" DIRECTORY)
set(_perm_src_dir "${_out_dir}/perm_src_prt_art")
set(_manifest "${_out_dir}/prt_art_permutations_manifest.txt")
file(MAKE_DIRECTORY "${_perm_src_dir}")

# PRT-ART-Achsen (Pruefling)
set(_PA_NODE "compact" "wide")                  # PA-1
set(_PA_PC   "none" "lazy")                     # PA-2 (Path-Compression)
set(_PA_LOOKUP "linear" "binary")               # PA-3
set(_PA_TELEM "leaf_only" "sampling")           # PA-4 (Kuehn 11.X1)

if(COMDARE_PROFILE STREQUAL "smoke")
    # 2x2x2x2 = 16 Permutationen
elseif(COMDARE_PROFILE STREQUAL "medium")
    list(APPEND _PA_NODE "leaf_only")
    list(APPEND _PA_PC   "eager")
    list(APPEND _PA_LOOKUP "simd")
    list(APPEND _PA_TELEM "offline")
    # 3x3x3x3 = 81
elseif(COMDARE_PROFILE STREQUAL "full")
    list(APPEND _PA_NODE "leaf_only" "internal")
    list(APPEND _PA_PC   "eager")
    list(APPEND _PA_LOOKUP "simd")
    list(APPEND _PA_TELEM "offline")
endif()

set(_all_perms "")
foreach(_n IN LISTS _PA_NODE)
    foreach(_p IN LISTS _PA_PC)
        foreach(_l IN LISTS _PA_LOOKUP)
            foreach(_t IN LISTS _PA_TELEM)
                list(APPEND _all_perms "${_n}|${_p}|${_l}|${_t}")
            endforeach()
        endforeach()
    endforeach()
endforeach()

list(LENGTH _all_perms _n_perms)
message(STATUS "PRT-ART Permutations-Codegen: ${_n_perms} gueltige Permutationen (Profile=${COMDARE_PROFILE})")

set(_count_written 0)
set(_count_skipped 0)
foreach(_perm IN LISTS _all_perms)
    string(REPLACE "|" "_" _perm_id "${_perm}")
    set(_wrapper "${_perm_src_dir}/perm_pa_${_perm_id}.cpp")
    if(EXISTS "${_wrapper}" AND NOT COMDARE_MODE STREQUAL "on_rebuild")
        math(EXPR _count_skipped "${_count_skipped} + 1")
    else()
        string(REPLACE "|" ";" _parts "${_perm}")
        list(GET _parts 0 _node)
        list(GET _parts 1 _pc)
        list(GET _parts 2 _lookup)
        list(GET _parts 3 _telem)
        file(WRITE "${_wrapper}"
"// Auto-generiert von prt_art/permutations_codegen/codegen.cmake (V36.C 2026-05-23)
// PRT-ART Pruefling-Permutation: ${_perm_id}
// Profile=${COMDARE_PROFILE}

#define COMDARE_PA_PERM_ID \"pa_${_perm_id}\"
#define COMDARE_PA_NODE \"${_node}\"
#define COMDARE_PA_PATH_COMPRESSION \"${_pc}\"
#define COMDARE_PA_LOOKUP \"${_lookup}\"
#define COMDARE_PA_TELEMETRY \"${_telem}\"

namespace comdare::prt_art::perm::pa_${_perm_id} {

extern \"C\" const char* perm_pa_${_perm_id}_id() {
    return COMDARE_PA_PERM_ID;
}

const char* pa_axis_node     = COMDARE_PA_NODE;
const char* pa_axis_pc       = COMDARE_PA_PATH_COMPRESSION;
const char* pa_axis_lookup   = COMDARE_PA_LOOKUP;
const char* pa_axis_telem    = COMDARE_PA_TELEMETRY;

}  // namespace
")
        math(EXPR _count_written "${_count_written} + 1")
    endif()
endforeach()

message(STATUS "PRT-ART Permutations-Codegen: ${_count_written} Wrapper geschrieben, ${_count_skipped} on-demand uebersprungen")

# Manifest
set(_manifest_content "# prt_art_permutations_manifest.txt (V36.C)\n# Profile=${COMDARE_PROFILE} Mode=${COMDARE_MODE} count=${_n_perms}\n")
foreach(_perm IN LISTS _all_perms)
    string(REPLACE "|" "_" _perm_id "${_perm}")
    string(APPEND _manifest_content "perm_pa_${_perm_id}\n")
endforeach()
file(WRITE "${_manifest}" "${_manifest_content}")

# permutations.cmake mit add_library + custom_target Aggregator
set(_cmake_content
"# prt_art_permutations.cmake — generiert von prt_art/permutations_codegen/codegen.cmake
# V36.C Vollausbau: ${_n_perms} Pruefling-Permutationen
# Profile=${COMDARE_PROFILE} Mode=${COMDARE_MODE}

message(STATUS \"PRT-ART Permutations: registriere ${_n_perms} Pruefling-Targets\")

")

foreach(_perm IN LISTS _all_perms)
    string(REPLACE "|" "_" _perm_id "${_perm}")
    string(APPEND _cmake_content
"add_library(perm_pa_${_perm_id} STATIC \"${_perm_src_dir}/perm_pa_${_perm_id}.cpp\")
target_compile_features(perm_pa_${_perm_id} PRIVATE cxx_std_23)
set_target_properties(perm_pa_${_perm_id} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY \"\${CMAKE_BINARY_DIR}/perm/prt_art/${_perm_id}\"
    FOLDER \"perm_prt_art\")

")
endforeach()

string(APPEND _cmake_content
"add_custom_target(comdare_prt_art_permutations_all
    COMMENT \"V36.C Aggregator: ${_n_perms} PRT-ART Pruefling-Permutationen\"
    DEPENDS")
foreach(_perm IN LISTS _all_perms)
    string(REPLACE "|" "_" _perm_id "${_perm}")
    string(APPEND _cmake_content "
        perm_pa_${_perm_id}")
endforeach()
string(APPEND _cmake_content "
)
")

file(WRITE "${COMDARE_OUTPUT}" "${_cmake_content}")
message(STATUS "PRT-ART Permutations-Codegen: ${COMDARE_OUTPUT} geschrieben (${_n_perms} Targets)")
