# comdare-prt-art — PRT-ART Pruefling, konsumiert comdare-cache-engine als Werkzeug
# REV 6 (User-Direktive 2026-05-12) — PRT-ART ist Pruefling, CacheEngine ist Werkzeug

cmake_minimum_required(VERSION 3.28)
project(comdare-prt-art
        VERSION 0.1.0
        DESCRIPTION "PRT-ART: Probst Redirect Tree / ART hybrid (experimenteller Suchalgorithmus-Pruefling)"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─────────────────────────────────────────────────────────────────────────────
# CacheEngine als Werkzeug — V41.E6 (2026-05-29): KEIN nested Submodule mehr.
# Plugin-Controller-Modell (Doku 20): die cache-engine LÄDT prt-art als Plugin
# (cache-engine -DCOMDARE_CE_PRUEFLINGE=<prt-art>) und stellt dabei ihre Includes
# selbst bereit — prt-arts CMakeLists ist dann NICHT der Build-Treiber. Für den
# prt-art-STANDALONE-Build zeigt COMDARE_CACHE_ENGINE_DIR auf die Geschwister-
# cache-engine (Default) bzw. einen vom Aufrufer via -D gesetzten Pfad.
# (Hebt die nested-Submodule-Direktive 2026-05-12 auf; E6/#22.)
# ─────────────────────────────────────────────────────────────────────────────
set(COMDARE_CACHE_ENGINE_DIR "${CMAKE_SOURCE_DIR}/../comdare-cache-engine"
    CACHE PATH "Pfad zur comdare-cache-engine (Geschwister-Checkout; nur Standalone-Build)")

# Aktuelle cache-engine-Include-Struktur (V41.F.x): libs/cache_engine/include
set(COMDARE_CE_INCLUDE_DIR "${COMDARE_CACHE_ENGINE_DIR}/libs/cache_engine/include")
if(NOT EXISTS "${COMDARE_CE_INCLUDE_DIR}/cache_engine/concepts/permutation_flags.hpp")
    message(WARNING
        "comdare-cache-engine nicht unter COMDARE_CACHE_ENGINE_DIR gefunden.\n"
        "prt-art-STANDALONE-Build benoetigt die cache-engine-Header unter:\n"
        "  ${COMDARE_CE_INCLUDE_DIR}\n"
        "Setze -DCOMDARE_CACHE_ENGINE_DIR=<pfad>, ODER baue prt-art als Plugin via\n"
        "  cache-engine -DCOMDARE_CE_PRUEFLINGE=<prt-art>  (dann liefert der Controller die Includes).")
endif()

option(COMDARE_PRT_ART_BUILD_TESTS "Build Google-Tests" ON)
option(COMDARE_PRT_ART_USE_CACHE_ENGINE "CacheEngine als add_subdirectory einbinden" OFF)

# V36.C (2026-05-23): Pre-Build Pruefling-Permutations
option(COMDARE_PRT_ART_BUILD_PERMUTATIONS "Pre-Build PRT-ART Pruefling-Algorithmus-Rekombinationen" OFF)

if(COMDARE_PRT_ART_BUILD_PERMUTATIONS)
    include(cmake/permutations.cmake OPTIONAL)
    if(EXISTS "${CMAKE_BINARY_DIR}/generated/prt_art_permutations.cmake")
        include("${CMAKE_BINARY_DIR}/generated/prt_art_permutations.cmake")
    endif()
endif()

if(COMDARE_PRT_ART_USE_CACHE_ENGINE)
    add_subdirectory(${COMDARE_CACHE_ENGINE_DIR}
                     ${CMAKE_BINARY_DIR}/cache_engine_external
                     EXCLUDE_FROM_ALL)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# PRT-ART Concept-Library (header-only)
# ─────────────────────────────────────────────────────────────────────────────
add_library(comdare_prt_art_core INTERFACE)
add_library(comdare::prt_art_core ALIAS comdare_prt_art_core)
target_include_directories(comdare_prt_art_core INTERFACE
    "${CMAKE_SOURCE_DIR}/prt_art/include"
    "${COMDARE_CE_INCLUDE_DIR}"                  # V41.E6: <cache_engine/...> (libs/cache_engine/include)
    "${COMDARE_CACHE_ENGINE_DIR}/libs/cache_engine")  # V41.F.6 Phase B: <topics/...> + <anatomy/...> für Slot-Header
target_compile_features(comdare_prt_art_core INTERFACE cxx_std_23)

# ─────────────────────────────────────────────────────────────────────────────
# Multi-OS-Defines (analog comdare-cache-engine/cmake/platform_detection.cmake)
# ─────────────────────────────────────────────────────────────────────────────
if(WIN32)
    target_compile_definitions(comdare_prt_art_core INTERFACE COMDARE_OS_WINDOWS=1)
elseif(APPLE)
    target_compile_definitions(comdare_prt_art_core INTERFACE COMDARE_OS_MACOS=1)
elseif(UNIX)
    target_compile_definitions(comdare_prt_art_core INTERFACE COMDARE_OS_LINUX=1)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Tests
# ─────────────────────────────────────────────────────────────────────────────
if(COMDARE_PRT_ART_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    set(_gtest_local "${COMDARE_CACHE_ENGINE_DIR}/cmake/third_party/googletest-1.15.2.tar.gz")
    if(EXISTS "${_gtest_local}")
        FetchContent_Declare(googletest URL "${_gtest_local}")
    else()
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
            URL_HASH SHA256=7b42b4d6ed48810c5362c265a17faebe90dc2373c885e5216439d37927f02926
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    endif()
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests/unit)
endif()

# REV 7.6 V9.4 — PRT-ART Pruefling-Re-Implementations (verschoben aus cache-engine)
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/prt_art/legacy_reimpl/CMakeLists.txt")
    add_subdirectory(prt_art/legacy_reimpl)
endif()

message(STATUS "──────────────────────────────────────────────────────────")
message(STATUS "comdare-prt-art ${PROJECT_VERSION}")
message(STATUS "  C++ Standard            : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Cache-Engine Werkzeug   : ${COMDARE_CACHE_ENGINE_DIR}")
message(STATUS "  Use Cache-Engine sub    : ${COMDARE_PRT_ART_USE_CACHE_ENGINE}")
message(STATUS "  Build Tests             : ${COMDARE_PRT_ART_BUILD_TESTS}")
message(STATUS "  Build Legacy-Reimpl V9.4: ${COMDARE_PRT_ART_BUILD_LEGACY_REIMPL}")
message(STATUS "──────────────────────────────────────────────────────────")
