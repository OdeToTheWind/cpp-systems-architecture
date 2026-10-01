# Day 25 – Local Development Environment Setup Reflection

**Date:** 2026-04-07 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_25_dev_environment/lesson.hpp`](../../src/day_25_dev_environment/lesson.hpp) · **Tests:** [`tests/test_day_25.cpp`](../../tests/test_day_25.cpp) (9 tests)

## Scenario
A *project doctor* that examines a C++ checkout – this repository by default – and reports whether the local setup follows best practice: a CMake build, presets for development and sanitizers, strict warnings, a formatter configuration, an ignored build folder, and a modern compiler and language standard.

## Syllabus deliverables
> Compilers, CMake presets, warnings as errors, sanitizers, a reproducible project layout

| Deliverable | Implemented in |
|---|---|
| ✅ identifying the compiler and standard from predefined macros | `detect_compiler` |
| ✅ reading CMake preset names | `preset_names` |
| ✅ checking that warnings can be made errors | `check_warnings` |
| ✅ checking for a sanitizer configuration | `check_sanitizers` |
| ✅ checking the project layout | `check_layout` |
| ✅ one report combining every check | `diagnose` |

## Key learnings
- `CMakePresets.json` turns long configure commands into named, shareable presets (`cmake --preset dev`), so everyone builds the same way.
- Warnings are cheap bug reports: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` plus `-Werror` in CI keeps them from piling up.
- AddressSanitizer and UBSan find memory errors and undefined behaviour at run time, which ordinary tests can pass right over.
- Predefined macros (`__clang__`, `__GNUC__`, `_MSC_VER`, `__cplusplus`) tell code which compiler and standard it is built with – MSVC needs `/Zc:__cplusplus` to report the real standard.

## Pitfalls I hit (and how I fixed them)
- The old CI only built the demos and never ran the tests; the project doctor now checks the layout and CTest runs every suite.
- `message("Configured ${CMAKE_ARGC} day executables")` printed nothing useful – the build now counts the days it actually configures.
- MSVC reported `__cplusplus` as 199711 although C++20 was enabled, until `/Zc:__cplusplus` was added to the project options.

## Run it
```bash
./procpp.sh 25                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_25_dev_environment       # the interactive demo
ctest --test-dir build -R test_day_25 --output-on-failure
```

## Next step
- Day 26 makes the editor do the navigation and refactoring work.
