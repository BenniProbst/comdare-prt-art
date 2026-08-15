# V41.E11 Phase B -- prt-art als cache-engine-Plugin (Plugin-Controller-Registrierungs-Snippet).
#
# Wird von der cache-engine inkludiert, wenn prt-art via COMDARE_CE_PRUEFLINGE geladen wird
# (Doku 20 Plugin-Controller). Laeuft im Scope der cache-engine-Root-CMakeLists, VOR
# add_subdirectory(tests) -- daher propagieren die Listen-Variablen in tests/unit.
#
# W-B (2026-08-15): ANSCHLUSS AN DEN W0a-KONTRAKT (ce cmake/pruefling_kontrakt.cmake, 2026-08-10,
# "ZUSICHERUNG STATT ANWESENHEIT"). Der Plugin-Controller der ce-Root-CMakeLists verlangt seit
# W0a/S1, dass dieses Snippet comdare_pruefling_deklarieren() aufruft (REGEL 2: ein Snippet, das
# laeuft und nichts zusichert, ist kein Pruefling -> FATAL_ERROR). Die Vorfassung dieses Snippets
# (nur list(APPEND COMDARE_PRUEFLING_INCLUDE_DIRS/TEST_SOURCES ...)) lag 5 Tage hinter dem
# Kontrakt: jeder echte COMDARE_CE_PRUEFLINGE-Load brach damit im Configure ab.
#
# Zusicherung pruefling_slots_v1: prt-art liefert Achsen-Slots nach
# libs/cache_engine/anatomy/pruefling_merge.hpp (PrueflingVariants + has_pruefling). Der BELEG
# steht HINTER der Zusicherung und kann sie nur widerlegen (ROT), nie ein Gatter oeffnen.
# Die Laufzeit-Registrierung bleibt comdare::prt_art::pruefling::register_prt_art_pruefling(reg).

comdare_pruefling_deklarieren(
    NAME         prt-art
    FAEHIGKEITEN pruefling_slots_v1
    INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/prt_art/include"
    TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/prt_art/tests/unit/test_prt_art_pruefling_registration.cpp"
    BELEG        prt_art/slots/axis_01_page_type_slot.hpp
                 prt_art/slots/axis_07_prefetch_slot.hpp
                 prt_art/slots/axis_11_telemetry_slot.hpp
                 prt_art/slots/axis_14_value_handle_slot.hpp)

# ALT-PFAD-ADAPTER -- FOERMLICHE STILLLEGUNG (W-B, 2026-08-15; Option "Stilllegung mit Kommentar"):
# prt_art/include/prt_art/identity/prt_art_execution_engine_adapter.hpp (QUARANTAENE seit
# AP-2-neu/#236 W4, 2026-07-07) inkludiert "cache_engine/builder/commands/execute_engine_command.hpp".
# Dieser Include loest NUR mit <cache-engine>/libs als Include-Wurzel auf; diese Wurzel wird hier
# BEWUSST NICHT deklariert. Am Objekt gemessen (prt-art 9bcf887, 2026-08-15): 0 inkludierende
# Quellen und 0 CMake-Verdrahtungen fuer prt_art_execution_engine_adapter im ganzen Repo
# (Gegenprobe: das QUARANTAENE-Banner der Datei selbst trifft). Der Adapter ist Alt-Pfad, NICHT
# Teil der Zusicherung pruefling_slots_v1 und darf nicht in den Mess-Pfad verdrahtet werden.
# Reaktivierung nur als eigene, neue Faehigkeit im ce-Kontrakt
# (COMDARE_PRUEFLING_BEKANNTE_FAEHIGKEITEN) mit eigenem BELEG -- nicht durch stilles Anhaengen.
