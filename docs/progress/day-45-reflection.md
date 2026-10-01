# Day 45 – STL Algorithms Reflection

**Date:** 2026-04-27 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_45_stl_algorithms/lesson.hpp`](../../src/day_45_stl_algorithms/lesson.hpp) · **Tests:** [`tests/test_day_45.cpp`](../../tests/test_day_45.cpp) (8 tests)

## Scenario
An *e-sports league table*. Season results are turned into points, players under the minimum number of matches are filtered out, disqualified players are removed, the table is sorted with proper tie-breakers and split into promotion and relegation zones – each step one standard algorithm instead of a hand-written loop.

## Syllabus deliverables
> transform, copy\_if, accumulate, sort with custom comparators, partition, erase-remove

| Deliverable | Implemented in |
|---|---|
| ✅ std::transform turns records into derived values | `points_of` |
| ✅ std::copy\_if keeps only qualifying players | `qualified` |
| ✅ std::accumulate folds a range into one value | `total_points` |
| ✅ erase-remove (and std::erase\_if) deletes in one pass | `remove_disqualified` |
| ✅ std::sort with a multi-key comparator | `rank` |
| ✅ std::stable\_partition splits into zones | `promotion_zone` |

## Key learnings
- `std::transform`, `std::copy_if` and `std::accumulate` name *what* a loop does, which makes intent obvious and off-by-one errors impossible.
- `std::remove_if` only moves the kept elements forward; `erase` must cut off the tail – or use C++20 `std::erase_if` directly.
- Comparing `std::tuple`s gives a lexicographic multi-key ordering for free; negating numbers turns ascending into descending.
- `std::stable_partition` splits a range by a predicate while keeping the relative order inside each part.

## Pitfalls I hit (and how I fixed them)
- Calling `remove_if` without `erase` left the vector the same size with leftover elements at the end.
- Players tied on points were ordered differently on each run with a custom comparator that ignored names; the name is now the final tie-breaker.
- `std::partition` scrambled the ranking inside the promotion zone; `std::stable_partition` keeps it.

## Run it
```bash
./procpp.sh 45                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_45_stl_algorithms       # the interactive demo
ctest --test-dir build -R test_day_45 --output-on-failure
```

## Next step
- Day 46 writes functions that accept any number of arguments of any type.
