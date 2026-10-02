# Day 66 – Ranges & Views Reflection

**Date:** 2026-05-18 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_66_ranges/lesson.hpp`](../../src/day_66_ranges/lesson.hpp) · **Tests:** [`tests/test_day_66.cpp`](../../tests/test_day_66.cpp) (7 tests)

## Scenario
*order analytics for a small online shop*. Questions such as "what did paid orders over €50 earn?", "who are the top three customers?" or "which tags appear most?" are answered with range algorithms and lazy view pipelines instead of hand-written loops and temporary vectors.

## Syllabus deliverables
> Range algorithms, lazy views, composing pipelines, projections

| Deliverable | Implemented in |
|---|---|
| ✅ a filter-and-transform pipeline over orders | `paid_amounts` |
| ✅ sorting with a projection instead of a comparator | `sort_by_total` |
| ✅ taking the first n results lazily | `top_customers` |
| ✅ proving that views are lazy | `count_evaluations` |
| ✅ splitting text with a view | `split_tags` |

## Key learnings
- Range algorithms take the whole container (`std::ranges::sort(v)`), so begin/end mismatches cannot happen.
- Projections say what to compare – `&Order::cents` – so most custom comparators disappear.
- Views are lazy: a pipeline of filter, transform and take does work only for the elements actually read, so even an infinite `iota` is fine.
- `views::split` and `views::join` turn text processing into pipelines without temporary vectors.

## Pitfalls I hit (and how I fixed them)
- A filter placed after a transform calls the transform twice per accepted element – once to test and once to read – which surprised a call counter.
- Views hold references to their source; returning a view over a local vector would dangle, so results are copied into a vector before returning.
- `std::ranges::to` is C++23, so pipelines are materialised with the iterator-pair constructor instead.

## Run it
```bash
./procpp.sh 66                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_66_ranges       # the interactive demo
ctest --test-dir build -R test_day_66 --output-on-failure
```

## Next step
- Day 67 looks at who owns objects: smart pointers.
