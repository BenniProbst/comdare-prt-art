# tests/unit/CMakeLists.txt — comdare-prt-art Tests

include(GoogleTest)

function(comdare_prt_art_add_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBRARIES" ${ARGN})
    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE
        gtest gtest_main
        ${ARG_LIBRARIES})
    target_compile_features(${name} PRIVATE cxx_std_23)
    gtest_discover_tests(${name}
        DISCOVERY_TIMEOUT 30
        PROPERTIES TIMEOUT 60)
endfunction()

comdare_prt_art_add_test(test_prt_art_nodes
    SOURCES test_prt_art_nodes.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_internal_search
    SOURCES test_internal_search.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_value_handle
    SOURCES test_value_handle.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_memory_layout
    SOURCES test_memory_layout.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_allocator_pools
    SOURCES test_allocator_pools.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_olc_with_reserved_blocks
    SOURCES test_olc_with_reserved_blocks.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_prefetch_strategies
    SOURCES test_prefetch_strategies.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_measurement
    SOURCES test_measurement.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_prt_art_identity
    SOURCES test_prt_art_identity.cpp
    LIBRARIES comdare::prt_art_core)

comdare_prt_art_add_test(test_serialization_and_buffer
    SOURCES test_serialization_and_buffer.cpp
    LIBRARIES comdare::prt_art_core)

# Lizenz-Wache (2026-08-14) - Port der ce-Wache test_lizenz_konsistenz (P6-Form, ce f6d13dfb).
# Prueft den Zustand DIESES Repos: ein Bezeichner (LICENSE/NOTICE/SPDX-Koepfe), Change-Date-
# Feld + wahre prt-art-Historie, OF-1-ISO-Gate. Wurzel via PROJECT_SOURCE_DIR (Muster-D/#14:
# CMAKE_SOURCE_DIR zeigte im super-Sub-Build auf die Superprojekt-Wurzel).
comdare_prt_art_add_test(test_lizenz_konsistenz
    SOURCES test_lizenz_konsistenz.cpp)
target_compile_definitions(test_lizenz_konsistenz PRIVATE
    "COMDARE_LIZENZ_REPO_WURZEL=\"${PROJECT_SOURCE_DIR}\"")

# V33.B.1 (2026-05-21) - DefaultLookupRegistry-Tests
add_executable(test_default_lookup_registry test_default_lookup_registry.cpp)
target_link_libraries(test_default_lookup_registry PRIVATE gtest gtest_main)
target_include_directories(test_default_lookup_registry PRIVATE
    ${CMAKE_SOURCE_DIR}/prt_art/include)
target_compile_features(test_default_lookup_registry PRIVATE cxx_std_23)
gtest_discover_tests(test_default_lookup_registry DISCOVERY_TIMEOUT 30)

# V34.E.2 (2026-05-21) - LeafOnlyCounter (Kuehn 11.X1) Tests
add_executable(test_leaf_only_counter test_leaf_only_counter.cpp)
target_link_libraries(test_leaf_only_counter PRIVATE gtest gtest_main)
target_include_directories(test_leaf_only_counter PRIVATE
    ${CMAKE_SOURCE_DIR}/prt_art/include)
target_compile_features(test_leaf_only_counter PRIVATE cxx_std_23)
gtest_discover_tests(test_leaf_only_counter DISCOVERY_TIMEOUT 30)
