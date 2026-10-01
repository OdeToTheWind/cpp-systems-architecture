# Day 44 – Tabular Data Analysis Reflection

**Date:** 2026-04-26 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_44_tabular_data/lesson.hpp`](../../src/day_44_tabular_data/lesson.hpp) · **Tests:** [`tests/test_day_44.cpp`](../../tests/test_day_44.cpp) (7 tests)

## Scenario
A *bike-share trip analysis*. A month of trips is loaded into a small column-oriented table – the same idea as a pandas DataFrame – then filtered, extended with a derived speed column, grouped by station and summarised with count, mean, median and standard deviation.

## Syllabus deliverables
> Column-oriented tables, filtering, derived columns, group-by aggregation, summary statistics

| Deliverable | Implemented in |
|---|---|
| ✅ a column-oriented table with typed columns | `Table` |
| ✅ filtering rows with a predicate | `Table::filter` |
| ✅ adding a column derived from others | `Table::derive` |
| ✅ group-by with an aggregation | `group_mean` |
| ✅ count, mean, median, standard deviation, min and max | `describe` |

## Key learnings
- A column-oriented table stores each column contiguously, which makes scanning one column (sum, mean, filter) cache-friendly.
- Filtering produces a new table with the selected row indices applied to every column, so the columns stay aligned.
- Group-by is a map from key to running aggregates (sum and count), finished by dividing once at the end.
- The sample standard deviation divides by n − 1; the median needs sorted data and averages the two middle values for even counts.

## Pitfalls I hit (and how I fixed them)
- Adding a column with the wrong number of rows silently misaligned the table; every new column is now checked against the row count.
- Very short trips (docking errors) produced absurd speeds; they are filtered out before grouping.
- The median of an even-sized column returned the upper middle value until both middle values were averaged.

## Run it
```bash
./procpp.sh 44                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_44_tabular_data       # the interactive demo
ctest --test-dir build -R test_day_44 --output-on-failure
```

## Next step
- Day 45 replaces hand-written loops with standard algorithms.
