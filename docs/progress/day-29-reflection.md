# Day 29 – Using External Libraries Reflection

**Date:** 2026-04-11 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_29_external_libraries/lesson.hpp`](../../src/day_29_external_libraries/lesson.hpp) · **Tests:** [`tests/test_day_29.cpp`](../../tests/test_day_29.cpp) (8 tests)

## Scenario
A *newsletter editor's text toolkit* built on a library called textkit, which ships in two flavours exactly like real dependencies do: a header-only part (just include it) and a compiled static library (CMake target `textkit`, linked by this lesson). A dependency checker reads a manifest, compares semantic versions and prints the CMake you would write to get each library with find_package or FetchContent.

## Syllabus deliverables
> Header-only vs compiled libraries, linking a static library, find\_package and FetchContent, version checks

| Deliverable | Implemented in |
|---|---|
| ✅ a header-only library: include and use | `headline` |
| ✅ a compiled static library: declare, link, call | `summarise` |
| ✅ semantic versions that compare correctly | `Version` |
| ✅ version requirements: >=, ^ and == | `satisfies` |
| ✅ checking a manifest against installed versions | `check_manifest` |
| ✅ the CMake for find\_package or FetchContent | `cmake_snippet` |

## Key learnings
- A header-only library is used by including it; a compiled library also needs its object code linked into the program.
- In CMake, `target_link_libraries(app PRIVATE textkit)` links the library and also hands over its include directories and other usage requirements.
- `find_package` uses a library already installed on the system, while `FetchContent` downloads and builds it during configuration.
- Version checks belong at both levels: `static_assert` on the header macros at compile time, and the linked library's `version()` at run time.

## Pitfalls I hit (and how I fixed them)
- Defining a non-inline function in a header caused 'multiple definition' link errors; header-only functions must be `inline`.
- Members named `major` and `minor` clashed with macros from a system header; they are now `major_version` and `minor_version`.
- Comparing versions as strings ranked 1.10.0 below 1.9.9; the `Version` struct compares numbers with a defaulted `<=>`.

## Run it
```bash
./procpp.sh 29                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_29_external_libraries       # the interactive demo
ctest --test-dir build -R test_day_29 --output-on-failure
```

## Next step
- Day 30 controls access to an object's data with getters and setters.
