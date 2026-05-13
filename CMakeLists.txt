# comdare-prt-art — PRT-ART Pruefling, konsumiert comdare-cache-engine als Werkzeug
# REV 6 (User-Direktive 2026-05-12) — PRT-ART ist Pruefling, CacheEngine ist Werkzeug

cmake_minimum_required(VERSION 3.28)
project(comdare-prt-art
        VERSION 0.1.0
        DESCRIPTION "PRT-ART: Permutations-Trie-ART (experimenteller Suchalgorithmus-Pruefling)"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─────────────────────────────────────────────────────────────────────────────
# CacheEngine als Werkzeug (Git-Submodule unter external/comdare-cache-engine,
# User-Direktive 2026-05-12, Ausnahme zu CLAUDE.md S2683 analog S2686b)
# ─────────────────────────────────────────────────────────────────────────────
set(COMDARE_CACHE_ENGINE_DIR "${CMAKE_SOURCE_DIR}/external/comdare-cache-engine"
    CACHE PATH "Pfad zur als Submodule eingebundenen comdare-cache-engine")

if(NOT EXISTS "${COMDARE_CACHE_ENGINE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "comdare-cache-engine Submodule nicht initialisiert.\n"
        "Bitte ausfuehren: git submodule update --init --recursive\n"
        "Erwarteter Pfad: ${COMDARE_CACHE_ENGINE_DIR}")
endif()

option(COMDARE_PRT_ART_BUILD_TESTS "Build Google-Tests" ON)
option(COMDARE_PRT_ART_USE_CACHE_ENGINE "CacheEngine als add_subdirectory einbinden" OFF)

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
    "${COMDARE_CACHE_ENGINE_DIR}/cache_engine/include"
    "${COMDARE_CACHE_ENGINE_DIR}/cache_engine/subsystems"
    "${COMDARE_CACHE_ENGINE_DIR}/prt_art/include")
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
