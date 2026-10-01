# Day 11 – Error Handling Reflection

**Date:** 2026-03-24 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_11_error_handling/lesson.hpp`](../../src/day_11_error_handling/lesson.hpp) · **Tests:** [`tests/test_day_11.cpp`](../../tests/test_day_11.cpp) (9 tests)

## Scenario
A *greenhouse sensor log reader*. Log files are messy, devices disappear and people type impossible values; the reader must keep going, count what went wrong and why, and only stop for errors it truly cannot handle.

## Syllabus deliverables
> throw, try and catch, the std::exception hierarchy, custom exceptions, rethrowing, input validation

| Deliverable | Implemented in |
|---|---|
| ✅ a custom exception derived from std::runtime\_error | `SensorError` |
| ✅ throwing standard exceptions for invalid input | `parse_reading` |
| ✅ catching by const reference, most specific first | `summarise_log` |
| ✅ rethrowing with throw; after logging | `load_with_audit` |
| ✅ bounds-checked access with .at() and std::out\_of\_range | `reading_at` |

## Key learnings
- Throw objects derived from `std::exception`, never strings or integers – callers can then catch one base class and still call `what()`.
- Catch by `const&`: catching by value copies the exception and slices a derived type down to its base.
- Order catch clauses from most specific to most general, because the first matching handler wins.
- `throw;` rethrows the exception currently being handled unchanged; `throw error;` would throw a sliced copy.

## Pitfalls I hit (and how I fixed them)
- A non-numeric divisor became 0 and was reported as division by zero; parsing now distinguishes malformed text (`std::invalid_argument`) from impossible values (`SensorError`).
- `std::stod("1e999")` throws `std::out_of_range`, which the first version did not catch – the log reader now handles it as malformed input.
- Indexing `readings[5]` past the end read garbage silently; `reading_at` uses `.at()`, which throws `std::out_of_range` instead.

## Run it
```bash
./procpp.sh 11                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_11_error_handling       # the interactive demo
ctest --test-dir build -R test_day_11 --output-on-failure
```

## Next step
- Day 12 splits the reader into small functions with clear parameters and return values.
