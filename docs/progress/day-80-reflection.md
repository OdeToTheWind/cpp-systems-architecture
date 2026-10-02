# Day 80 – Profiling & Performance Reflection

**Date:** 2026-06-01 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_80_performance/lesson.hpp`](../../src/day_80_performance/lesson.hpp) · **Tests:** [`tests/test_day_80.cpp`](../../tests/test_day_80.cpp) (7 tests)

## Scenario
The nightly *delivery-route analytics* job of a courier company has grown from minutes to hours. Before changing anything, it gets a benchmark harness that reports medians rather than single noisy runs. Then three classic wins are measured: a better algorithm (duplicate parcel IDs), a better data layout (structure of arrays) and a cache-friendly loop order.

## Syllabus deliverables
> Measuring with std::chrono, benchmark harnesses, data layout and cache locality, algorithmic wins

| Deliverable | Implemented in |
|---|---|
| ✅ a harness with warm-up runs, repetitions and a median | `benchmark` |
| ✅ stopping the optimiser from deleting measured work | `keep` |
| ✅ an O(n^2) and an O(n) duplicate check with operation counts | `has_duplicate_hashed` |
| ✅ array-of-structs versus struct-of-arrays layouts | `StopsSoA` |
| ✅ row-major versus column-major traversal | `sum_column_major` |

## Key learnings
- Measure with std::chrono::steady_clock, repeat the work, and report the median – a single run is mostly noise.
- The optimiser deletes work whose result is unused; writing results to a volatile sink keeps benchmarks honest.
- Algorithmic wins dominate: replacing an O(n²) duplicate check with a hash set turned millions of comparisons into thousands of lookups.
- Memory layout matters: struct-of-arrays and row-major loops read memory in order and use every byte the cache fetches.

## Pitfalls I hit (and how I fixed them)
- The first timed run included cache warm-up and lazy initialisation; the harness now does untimed warm-up runs.
- Asserting timings in unit tests made them flaky; tests check operation counts and use a fake clock instead.
- A function-local volatile sink triggered a set-but-unused warning; a namespace-scope inline variable works everywhere.

## Run it
```bash
./procpp.sh 80                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_80_performance       # the interactive demo
ctest --test-dir build -R test_day_80 --output-on-failure
```

## Next step
- Day 81 finds patterns in text with regular expressions.
