# registry_roundtrip.cmake — WP-4/F29 (Voll-Audit 2026-07-16): Round-Trip-Gate der prt-art-Baustein-
# Registry-XML (Muster R-E/M-CE-27; ANALOG ce tests/unit/registry_roundtrip.cmake — bewusst self-
# contained je Repo statt Cross-Repo-Referenz: der Geschwister-ce-Checkout kann in CI ein aelterer
# Stand sein, das Script muss zum prt-art-Stand versioniert bleiben).
#
# PROBLEM (F29): die committete prt_art_axis_registry.xml deklariert "GENERIERT ... NICHT von Hand
# editieren", aber der Generator (prt_art_axis_registry_gen, EXCLUDE_FROM_ALL) lief in KEINEM
# ctest/CI-Job — Slot-/Wrapper-Aenderungen driften stumm an der committeten XML vorbei. Dieses Gate:
#   1. Das VOR-gebaute Generator-Tool (GEN_TOOL) regeneriert die XML nach WORKDIR.
#   2. Byte-Diff gegen die committete Datei (COMMITTED) — jede Abweichung == FAIL.
# Die committete Datei wird NUR GELESEN; geschrieben wird ausschliesslich unter WORKDIR.
#
# REFERENZ-KONFIGURATION: prt-art-Standalone (Linux g++/Ninja) + ein KONFIGURIERTER Default-
# Standalone-Build der Geschwister-cache-engine (-DCOMDARE_CE_GENERATED_DIR=<ce-build>/generated,
# liefert die CMake-generierten *_flags.hpp des golden-verdrahteten Patricia-Merge-Organs). Der
# committete Byte-Stand stammt aus INC-B (2026-07-14), byte-verifiziert 2026-07-16. Ein abweichender
# ce-Geschwister-Stand (z.B. gedrifteter Patricia-Wrapper) schlaegt hier ehrlich an — das ist der
# gewollte Cross-Repo-Drift-Beweis, kein Fehler des Gates.
#
# Erwartete -D Variablen: GEN_TOOL (gebautes Generator-Executable), COMMITTED (committete XML),
# WORKDIR (Scratch-Verzeichnis).

foreach(_req GEN_TOOL COMMITTED WORKDIR)
    if(NOT DEFINED ${_req})
        message(FATAL_ERROR "registry_roundtrip: -D${_req} fehlt.")
    endif()
endforeach()

if(NOT EXISTS "${GEN_TOOL}")
    message(FATAL_ERROR
        "registry_roundtrip: Generator-Tool nicht gebaut: ${GEN_TOOL}\n"
        "Vorher bauen (2-Pass): cmake --build <build> --target prt_art_axis_registry_gen.")
endif()
if(NOT EXISTS "${COMMITTED}")
    message(FATAL_ERROR "registry_roundtrip: committete Registry-XML fehlt: ${COMMITTED}")
endif()

# Frisches Arbeitsverzeichnis (deterministisch); NUR hier wird geschrieben.
file(REMOVE_RECURSE "${WORKDIR}")
file(MAKE_DIRECTORY "${WORKDIR}")
set(_regen "${WORKDIR}/regenerated_registry.xml")

execute_process(
    COMMAND "${GEN_TOOL}" --out "${_regen}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _log
    ERROR_VARIABLE  _log_err)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "registry_roundtrip: Generator fehlgeschlagen (rc=${_rc}):\n${_log}\n${_log_err}")
endif()
message(STATUS "Generator-Ausgabe:\n${_log}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${_regen}" "${COMMITTED}"
    RESULT_VARIABLE _diff)
if(NOT _diff EQUAL 0)
    execute_process(COMMAND diff -u "${COMMITTED}" "${_regen}" OUTPUT_VARIABLE _diff_out ERROR_QUIET)
    message(FATAL_ERROR
        "registry_roundtrip: BYTE-DRIFT — die committete prt-art-Registry-XML entspricht NICHT der "
        "Reflektion des aktuellen Codes (Slot-/Wrapper-Aenderung ohne Regeneration? ce-Geschwister-Drift?).\n"
        "  committet:   ${COMMITTED}\n"
        "  regeneriert: ${_regen}\n"
        "Fix: Generator laufen lassen + XML mit-committen (NIE von Hand editieren); die ce-Fixture-KOPIE "
        "tests/unit/thesis_tiere/prt_art_axis_registry.xml hat Sync-Pflicht (super-Sync-Gate, F33/F66).\n${_diff_out}")
endif()
message(STATUS "registry_roundtrip: Byte-Diff == 0 — committete XML == Code-Reflektion.")
