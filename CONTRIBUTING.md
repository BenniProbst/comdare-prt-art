# Contributing

comdare-prt-art (PRT-ART: Probst Redirect Tree / ART hybrid) is the experimental
candidate module of the diploma thesis "Active Cache-Aware Hardware Adaptation:
Cache Engine for Trie-Based Index Structures" (Benjamin-Elias Probst, Technische
Universitaet Dresden, 2026). Rights holder of the code is BEP Venture UG
(haftungsbeschraenkt), trademark Comdare. The code is source-available under the
Comdare Research License 1.0 (see `LICENSE`): anyone may read, audit and review
it; research at an institution may build, run and modify it; business use and
individual use need a separate written license.

## Contributions and the license
- Bug reports, review findings and patches are welcome as issues and pull requests.
- By sending a patch you grant the Licensor the feedback license of `LICENSE`
  Section 7 (perpetual, irrevocable, royalty-free). Contributors keep the
  research-use rights of Section 3 like everyone else.
- Do not send code you are not allowed to license this way. This repository
  bundles no third-party code (`NOTICE`); keep it that way unless the license
  audit of the cache engine (`LICENSE_AUDIT_EXT.md` there) is extended first.

## Development Setup
- C++23, CMake 3.20+; GCC and Clang are both built in CI (dual-compiler rule).
- Standalone: `cmake -S . -B build && cmake --build build && ctest --test-dir build`
  (GoogleTest is fetched at configure time, preferably from the sibling
  checkout of comdare-cache-engine).
- As plugin: consumed by comdare-cache-engine via `comdare_pruefling.cmake`
  (`COMDARE_CE_PRUEFLINGE`); see `README.md`.

## Branches & Commits
- `development` is the integration branch, `main` the stable branch (Gitflow).
- Topic branches: `bau/<topic>`; merges are `--no-ff`; history is never rebased.
- Commit messages: imperative subject line, body explains WHY; ASCII-only.
- Decisions are booked in the umbrella ledger
  (`probst-diplomarbeit-cache-engine/docs/DIPLOMARBEIT-ZIELE-OFFENE-PUNKTE-LEDGER.md`);
  the repo-local view is `docs/ledger-sections/architektur-ziele-offene-punkte-ledger.md`.

## Tests & Style
- Google Test only, run in Debug and repeated in Release; shell probes are not tests.
- Every guard is built red-first: prove that it bites before you heal the tree,
  and keep the literal red output in the test's header.
- ASCII-only source and documentation, lines <= 120 bytes (`.clang-format`).

## Licensing information per file (REUSE 3.3)
- Every file needs REUSE-compliant licensing information: an SPDX header in the
  file or an entry in `REUSE.toml`. New own source files carry the house header
  (first two lines): the SPDX line with `LicenseRef-Comdare-Research-1.0` and
  `Copyright (c) 2026 BEP Venture UG (haftungsbeschraenkt), Marke Comdare`.
- The guard `tests/unit/test_lizenz_konsistenz.cpp` fails on any other SPDX
  identifier outside `docs/`.
- Run `reuse lint` at the repository root (for example `pipx run reuse lint` or
  `uvx --from reuse reuse lint`) before pushing.

## Pull Request Checklist
- [ ] README/docs updated; ledger entry added if a decision changes
- [ ] Tests added or adjusted, red-first proven, green in Debug and Release
- [ ] `CHANGELOG.md` updated if user-visible
- [ ] `reuse lint` green; secret scan (`.gitleaks.toml`) green
- [ ] CI pipeline green (GitLab and the GitHub mirror), no `allow_failure`
