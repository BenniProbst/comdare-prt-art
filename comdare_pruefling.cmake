# V41.E11 Phase B — prt-art als cache-engine-Plugin (Plugin-Controller-Registrierungs-Snippet).
#
# Wird von der cache-engine inkludiert, wenn prt-art via COMDARE_CE_PRUEFLINGE geladen wird
# (Doku 20 Plugin-Controller). Laeuft im Scope der cache-engine-Root-CMakeLists, VOR
# add_subdirectory(tests) — daher propagieren die Listen-Variablen in tests/unit.
#
# Contract: registriert (1) prt-art's Include-Pfad (fuer den Pruefling-Factory-Header) und
# (2) prt-art's Integration-Test bei der cache-engine. Die eigentliche Factory-Registrierung
# zur Laufzeit erfolgt via comdare::prt_art::pruefling::register_prt_art_pruefling(registry).

message(STATUS "comdare Plugin-Controller: prt-art Pruefling registriert (${CMAKE_CURRENT_LIST_DIR})")

list(APPEND COMDARE_PRUEFLING_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/prt_art/include")
list(APPEND COMDARE_PRUEFLING_TEST_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/prt_art/tests/unit/test_prt_art_pruefling_registration.cpp")
