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
