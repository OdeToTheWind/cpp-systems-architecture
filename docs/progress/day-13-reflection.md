# Day 13 – For Loops Reflection

**Date:** 2026-03-26 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_13_for_loops/lesson.hpp`](../../src/day_13_for_loops/lesson.hpp) · **Tests:** [`tests/test_day_13.cpp`](../../tests/test_day_13.cpp) (8 tests)

## Scenario
A *marathon timing station*. Chip mats record each runner's split at every checkpoint; loops total the times, rank the finishers while skipping runners who did not finish, find the first runner under a target time, and print a pace chart.

## Syllabus deliverables
> Counted and range-based for loops, nested loops, break and continue, index-safe iteration

| Deliverable | Implemented in |
|---|---|
| ✅ range-based for over a runner's splits | `total_seconds` |
| ✅ continue to skip runners who did not finish | `rank_finishers` |
| ✅ break as soon as the answer is found | `first_under` |
| ✅ nested counted loops build a table | `pace_chart` |
| ✅ counting down with an unsigned index without underflow | `splits_in_reverse` |

## Key learnings
- A range-based `for` over a container cannot run past its end, so it removes a whole class of off-by-one errors.
- `continue` skips the rest of one iteration and `break` leaves the loop; both keep early-exit logic flat.
- Index loops over containers should use `std::size_t` to match `size()` and avoid signed/unsigned comparison warnings.
- Counting down with an unsigned index is written `for (i = n; i-- > 0;)`, which tests before it decrements and never wraps to a huge value.

## Pitfalls I hit (and how I fixed them)
- `for (std::size_t i = n - 1; i >= 0; --i)` never ended, because an unsigned value is always >= 0 – the `i-- > 0` form fixed it.
- Ranking with `std::sort` shuffled runners with equal times; `std::stable_sort` keeps ties in start order.
- The pace chart overflowed `int` for long ultra distances in an early draft; the chart now stays within marathon distances and the test checks the largest value.

## Run it
```bash
./procpp.sh 13                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_13_for_loops       # the interactive demo
ctest --test-dir build -R test_day_13 --output-on-failure
```

## Next step
- Day 14 looks at the blocks these loops create and how indentation can lie about them.
