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
