# Day 33 – Namespaces Reflection

**Date:** 2026-04-15 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_33_namespaces/lesson.hpp`](../../src/day_33_namespaces/lesson.hpp) · **Tests:** [`tests/test_day_33.cpp`](../../tests/test_day_33.cpp) (7 tests)

## Scenario
A *weather-station data library* that ships two versions of its parser at once. Version 2 is the default through an inline namespace, version 1 stays available for old station firmware, unit helpers live in nested namespaces with a short alias, and two different `mean` functions coexist because each lives in its own namespace.

## Syllabus deliverables
> Nested and inline namespaces, namespace aliases, anonymous namespaces, using-declarations vs using-directives

| Deliverable | Implemented in |
|---|---|
| ✅ nested namespaces (C++17 a::b::c syntax) | `weather::units::metric` |
| ✅ an inline namespace makes v2 the default API | `weather::v2` |
| ✅ the old API kept reachable by name | `weather::v1` |
| ✅ a namespace alias shortens long names | `describe_reading` |
| ✅ two functions with one name in different namespaces | `daily_summary` |
| ✅ a using-declaration imports exactly one name | `wind_in_kmh` |

## Key learnings
- Namespaces group related names and prevent collisions: `stats::mean` and `climatology::mean` can live side by side.
- An `inline namespace` makes its contents available through the enclosing namespace, which is how libraries make the newest API version the default while keeping old versions reachable.
- A namespace alias (`namespace wu = weather::units;`) shortens long qualified names without importing anything.
- A using-declaration imports one name into a scope; a using-directive imports all of them and can make calls ambiguous – never put one in a header.

## Pitfalls I hit (and how I fixed them)
- `using namespace stats; using namespace climatology;` made `mean(values)` ambiguous; qualified names fixed it.
- An anonymous namespace in a header gives every including file its *own* copy of the names – fine for constants, wasteful for large objects.
- Old v1 firmware readings were silently parsed as v2 until the version was made explicit at the call site.

## Run it
```bash
./procpp.sh 33                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_33_namespaces       # the interactive demo
ctest --test-dir build -R test_day_33 --output-on-failure
```

## Next step
- Day 34 designs functions with optional parameters that stay readable at the call site.
