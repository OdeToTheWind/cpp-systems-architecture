# Day 95 – Performance-critical Module Reflection

**Date:** 2026-06-16 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_95_spatial_index/lesson.hpp`](../../src/day_95_spatial_index/lesson.hpp) · **Tests:** [`tests/test_day_95.cpp`](../../tests/test_day_95.cpp) (7 tests)

## Scenario
*nearest available driver* in a ride-hailing app. Scanning every driver for every ride request is simple and correct, but with 50,000 drivers and thousands of requests a second it is too slow. A uniform grid index answers the same question by looking only at nearby cells. The fast version is trusted only because it is cross-checked against the naive one on many random cases, and the speed-up is measured, not assumed.

## Syllabus deliverables
> Naive vs spatial-index nearest-neighbour search, benchmarks, cross-checked results

| Deliverable | Implemented in |
|---|---|
| ✅ the obviously-correct linear scan | `nearest_naive` |
| ✅ bucketing points into grid cells | `GridIndex` |
| ✅ ring search with a provable stopping rule | `GridIndex::nearest` |
| ✅ keeping the index current as drivers move | `GridIndex::move` |
| ✅ randomised cross-checking against the baseline | `cross_check` |

## Key learnings
- Keep the naive version: it is the specification the fast version is checked against.
- A uniform grid turns "nearest driver" into a search of nearby cells, growing in rings until no unvisited cell can contain anything closer.
- Randomised cross-checks over many seeds, cell sizes and queries – including points outside the city – catch the edge cases hand-written tests miss.
- Counting distance computations shows the speed-up deterministically; wall-clock timings are reported, never asserted.

## Pitfalls I hit (and how I fixed them)
- Stopping at the first ring that contained any driver missed closer drivers just across a cell border; the stopping rule now uses the ring's minimum possible distance.
- Ties between equally distant drivers made the two versions disagree; both now prefer the smaller id.
- std::normal_distribution differs between standard libraries, so tests assert properties of the data, not exact positions.

## Run it
```bash
./procpp.sh 95                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_95_spatial_index       # the interactive demo
ctest --test-dir build -R test_day_95 --output-on-failure
```

## Next step
- Day 96 turns a tool into a proper release.
