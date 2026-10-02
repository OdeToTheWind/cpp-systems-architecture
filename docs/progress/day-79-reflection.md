# Day 79 – Build Systems & Packaging Reflection

**Date:** 2026-05-31 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_79_build_packaging/lesson.hpp`](../../src/day_79_build_packaging/lesson.hpp) · **Tests:** [`tests/test_day_79.cpp`](../../tests/test_day_79.cpp) (7 tests)

## Scenario
*shipping an internal library*. Three in-house apps convert units with copy-pasted code; today it becomes "unitconv", a proper CMake package (see unitconv/CMakeLists.txt) with targets and usage requirements, install and export rules, a generated version header and a CPack configuration. This file models the rules the build system applies – semantic-version compatibility, build order, and how PUBLIC/PRIVATE/INTERFACE requirements propagate – so they can be tested, and it uses the real library through its public API.

## Syllabus deliverables
> CMake targets and usage requirements, install and export rules, versioning, CPack

| Deliverable | Implemented in |
|---|---|
| ✅ parsing MAJOR.MINOR.PATCH versions | `parse_version` |
| ✅ the SameMajorVersion rule used by find\_package | `compatible` |
| ✅ a dependency-ordered build with cycle detection | `build_order` |
| ✅ PUBLIC, PRIVATE and INTERFACE usage requirements | `effective_includes` |
| ✅ CPack's package file naming | `package_file_name` |

## Key learnings
- In modern CMake everything is a target, and usage requirements (include directories, compile features) travel with it as PUBLIC, PRIVATE or INTERFACE.
- install(TARGETS … EXPORT) plus a package config file let other projects use the library with find_package(unitconv 2.0 REQUIRED).
- The version is defined once, in project(VERSION), and flows into the generated header, the library's SOVERSION and the package version file.
- CPack turns the install rules into archives or installers; `cpack -G TGZ` produced unitconv-2.1.0-Linux.tar.gz.

## Pitfalls I hit (and how I fixed them)
- Exporting failed because the library linked a course-only helper target; wrapping it in $<BUILD_INTERFACE:…> keeps it out of the package.
- Hard-coded include paths broke after installation; $<BUILD_INTERFACE> and $<INSTALL_INTERFACE> give each tree the right path.
- Requesting version 3.0 must fail for a 2.1 library; SameMajorVersion compatibility enforces that.

## Run it
```bash
./procpp.sh 79                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_79_build_packaging       # the interactive demo
ctest --test-dir build -R test_day_79 --output-on-failure
```

## Next step
- Day 80 measures before optimising.
