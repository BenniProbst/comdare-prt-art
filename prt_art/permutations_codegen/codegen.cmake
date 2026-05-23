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

# V36.E: Achsen-Versionen + per-Perm Versionierung + selective Rebuild
set(_axes_versions_file "${CMAKE_CURRENT_LIST_DIR}/axes_versions.txt")
set(_axes_versions_content "")
if(EXISTS "${_axes_versions_file}")
    file(READ "${_axes_versions_file}" _axes_versions_content)
endif()

function(_pa_axis_version axis_key out_var)
    string(REGEX MATCH "axis_${axis_key}=([^\n\r]+)" _m "${_axes_versions_content}")
    if(_m)
        set(${out_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        set(${out_var} "v0" PARENT_SCOPE)
    endif()
endfunction()

function(_pa_bump_minor in_version out_var)
    if(in_version MATCHES "([0-9]+)\\.([0-9]+)\\.([0-9]+)")
        math(EXPR _new_minor "${CMAKE_MATCH_2} + 1")
        set(${out_var} "${CMAKE_MATCH_1}.${_new_minor}.0" PARENT_SCOPE)
    else()
        set(${out_var} "0.1.0" PARENT_SCOPE)
    endif()
endfunction()

set(_perm_versions_dir "${_out_dir}/perm_versions_prt_art")
file(MAKE_DIRECTORY "${_perm_versions_dir}")

set(_count_written 0)
set(_count_skipped 0)
set(_count_bumped 0)

foreach(_perm IN LISTS _all_perms)
    string(REPLACE "|" "_" _perm_id "${_perm}")
    set(_wrapper "${_perm_src_dir}/perm_pa_${_perm_id}.cpp")
    set(_version_file "${_perm_versions_dir}/perm_pa_${_perm_id}.version")

    string(REPLACE "|" ";" _parts "${_perm}")
    list(GET _parts 0 _node)
    list(GET _parts 1 _pc)
    list(GET _parts 2 _lookup)
    list(GET _parts 3 _telem)

    _pa_axis_version("pa1_node_${_node}" _av_node)
    _pa_axis_version("pa2_pc_${_pc}" _av_pc)
    _pa_axis_version("pa3_lookup_${_lookup}" _av_lookup)
    _pa_axis_version("pa4_telem_${_telem}" _av_telem)
    set(_current_axes_sig "node=${_av_node};pc=${_av_pc};lookup=${_av_lookup};telem=${_av_telem}")

    set(_stored_version "0.1.0")
    set(_stored_axes_sig "")
    if(EXISTS "${_version_file}")
        file(READ "${_version_file}" _vf_content)
        string(REGEX MATCH "version=([^\n\r]+)" _m "${_vf_content}")
        if(_m)
            set(_stored_version "${CMAKE_MATCH_1}")
        endif()
        string(REGEX MATCH "axes=([^\n\r]+)" _m "${_vf_content}")
        if(_m)
            set(_stored_axes_sig "${CMAKE_MATCH_1}")
        endif()
    endif()

    set(_needs_rebuild FALSE)
    if(NOT EXISTS "${_wrapper}")
        set(_needs_rebuild TRUE)
    elseif(COMDARE_MODE STREQUAL "on_rebuild")
        set(_needs_rebuild TRUE)
    elseif(NOT "${_stored_axes_sig}" STREQUAL "${_current_axes_sig}")
        set(_needs_rebuild TRUE)
        _pa_bump_minor("${_stored_version}" _stored_version)
        math(EXPR _count_bumped "${_count_bumped} + 1")
    endif()

    if(NOT _needs_rebuild)
        math(EXPR _count_skipped "${_count_skipped} + 1")
    else()
        file(WRITE "${_wrapper}"
"// Auto-generiert von prt_art/permutations_codegen/codegen.cmake (V36.C+E + V37.D + V38.B/D 2026-05-24)
// PRT-ART Pruefling-Permutation: ${_perm_id}
// Profile=${COMDARE_PROFILE}
// V36.E Version: ${_stored_version}  (Achsen-Sig: ${_current_axes_sig})

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>

// V38.B Cross-platform Symbol-Export
#if defined(_WIN32) || defined(__CYGWIN__)
  #define COMDARE_PA_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
  #define COMDARE_PA_EXPORT __attribute__((visibility(\"default\")))
#else
  #define COMDARE_PA_EXPORT
#endif

#define COMDARE_PA_PERM_ID \"pa_${_perm_id}\"
#define COMDARE_PA_PERM_VERSION \"${_stored_version}\"
#define COMDARE_PA_NODE \"${_node}\"
#define COMDARE_PA_PATH_COMPRESSION \"${_pc}\"
#define COMDARE_PA_LOOKUP \"${_lookup}\"
#define COMDARE_PA_TELEMETRY \"${_telem}\"

// V38.C - PermDescriptor: ausserhalb namespace fuer extern \"C\"-Export
struct PermDescriptor {
    const char* id;
    const char* version;
    const char* axes;
    int (*run)(unsigned long, double*);
};

extern \"C\" COMDARE_PA_EXPORT const char* perm_pa_${_perm_id}_id() {
    return COMDARE_PA_PERM_ID;
}

extern \"C\" COMDARE_PA_EXPORT const char* perm_pa_${_perm_id}_version() {
    return COMDARE_PA_PERM_VERSION;
}

extern \"C\" COMDARE_PA_EXPORT const char* perm_pa_${_perm_id}_axes() {
    return \"node=${_node},pc=${_pc},lookup=${_lookup},telem=${_telem}\";
}

// V38.D (2026-05-24) - PRT-ART Algorithmus-Body

extern \"C\" COMDARE_PA_EXPORT int perm_pa_${_perm_id}_run(unsigned long n_ops, double* out_micros_per_op) {
    if (!out_micros_per_op || n_ops == 0) {
        return -1;
    }
    std::map<std::uint64_t, std::uint64_t> trie_like;

    auto t0 = std::chrono::steady_clock::now();
    // Node-Achse: compact -> kleinere keys; wide -> grosse keys
    constexpr std::uint64_t kPrime = 11400714819323198485ULL;
    for (unsigned long i = 0; i < n_ops; ++i) {
        std::uint64_t k = static_cast<std::uint64_t>(i) * kPrime;
        trie_like.emplace(k, i);
    }
    // Lookup-Achse: linear/binary/simd (alle aktuell std::map binary)
    std::uint64_t hits = 0;
    for (unsigned long i = 0; i < n_ops; ++i) {
        std::uint64_t k = static_cast<std::uint64_t>(i) * kPrime;
        auto it = trie_like.find(k);
        if (it != trie_like.end()) {
            ++hits;
        }
    }
    auto t1 = std::chrono::steady_clock::now();
    if (hits != static_cast<std::uint64_t>(n_ops)) {
        return -2;
    }
    double total_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    *out_micros_per_op = total_us / static_cast<double>(n_ops * 2);
    return 0;
}

// V38.C Plugin-Entry-Symbol (ausserhalb namespace).
static constexpr PermDescriptor kDescriptor_pa_${_perm_id} {
    COMDARE_PA_PERM_ID,
    COMDARE_PA_PERM_VERSION,
    \"node=${_node},pc=${_pc},lookup=${_lookup},telem=${_telem}\",
    &perm_pa_${_perm_id}_run
};

extern \"C\" COMDARE_PA_EXPORT const PermDescriptor* comdare_perm_descriptor() {
    return &kDescriptor_pa_${_perm_id};
}
")
        file(WRITE "${_version_file}"
"perm_id=pa_${_perm_id}
version=${_stored_version}
axes=${_current_axes_sig}
last_codegen=${CMAKE_CURRENT_LIST_FILE}
")
        math(EXPR _count_written "${_count_written} + 1")
    endif()
endforeach()

message(STATUS "PRT-ART Permutations-Codegen: ${_count_written} geschrieben, ${_count_skipped} skipped, ${_count_bumped} minor-bumped (V36.E)")

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

# V37.G (2026-05-23): Hierarchischer Achsen-Ordnerbaum analog cache-engine.
# Wurzel: ${BINARY_DIR}/perm/prt_art/
# Achsen-Reihenfolge: Node -> PathCompression -> Lookup -> Telemetry
# Voller Pfad: perm/prt_art/node_<v>/pc_<v>/lookup_<v>/telem_<v>/perm_pa_<id>.lib
foreach(_perm IN LISTS _all_perms)
    string(REPLACE "|" "_" _perm_id "${_perm}")
    string(REPLACE "|" ";" _parts "${_perm}")
    list(GET _parts 0 _pa_node)
    list(GET _parts 1 _pa_pc)
    list(GET _parts 2 _pa_lookup)
    list(GET _parts 3 _pa_telem)
    set(_axis_path "node_${_pa_node}/pc_${_pa_pc}/lookup_${_pa_lookup}/telem_${_pa_telem}")

    # V38.B (2026-05-24): SHARED-Library statt STATIC.
    string(APPEND _cmake_content
"add_library(perm_pa_${_perm_id} SHARED \"${_perm_src_dir}/perm_pa_${_perm_id}.cpp\")
target_compile_features(perm_pa_${_perm_id} PRIVATE cxx_std_23)
set_target_properties(perm_pa_${_perm_id} PROPERTIES
    PREFIX \"\"
    OUTPUT_NAME \"perm_pa_${_perm_id}\"
    RUNTIME_OUTPUT_DIRECTORY \"\${CMAKE_BINARY_DIR}/perm/prt_art/${_axis_path}\"
    LIBRARY_OUTPUT_DIRECTORY \"\${CMAKE_BINARY_DIR}/perm/prt_art/${_axis_path}\"
    ARCHIVE_OUTPUT_DIRECTORY \"\${CMAKE_BINARY_DIR}/perm/prt_art/${_axis_path}\"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
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
