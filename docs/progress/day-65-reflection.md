# Day 65 – Concepts & Constraints Reflection

**Date:** 2026-05-17 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_65_concepts/lesson.hpp`](../../src/day_65_concepts/lesson.hpp) · **Tests:** [`tests/test_day_65.cpp`](../../tests/test_day_65.cpp) (7 tests)

## Scenario
A *dashboard widget library*. A widget shows a value or a series of values – an order count, a CPU percentage, a sequence of response times, or a domain object such as a server that knows its own label. Concepts state exactly which types each widget accepts, so a wrong type is rejected with a readable message, and constrained overloads pick the right formatting automatically.

## Syllabus deliverables
> Standard concepts, writing custom concepts, requires clauses, constrained overloads

| Deliverable | Implemented in |
|---|---|
| ✅ a function constrained with a standard concept | `clamp_to` |
| ✅ custom concepts built from requires expressions | `Labelled` |
| ✅ a concept for a numeric series | `Series` |
| ✅ overloads chosen by the most specific constraint | `format_value` |
| ✅ a constrained widget that renders any series | `sparkline` |

## Key learnings
- Concepts name the requirements of a template, so a wrong type fails at the call with a short message instead of deep inside the body.
- Standard concepts such as `std::integral`, `std::floating_point` and `std::totally_ordered` cover most needs; custom ones combine them with `requires` expressions.
- A requires expression can check the return type of a call: `{ t.label() } -> std::convertible_to<std::string>`.
- When several constrained overloads match, the more constrained one wins – `std::unsigned_integral` subsumes `std::integral`.

## Pitfalls I hit (and how I fixed them)
- `bool` and `char` count as integral, so the Numeric concept excludes them explicitly.
- Writing the same constraint as two unrelated custom concepts made the overload ambiguous; subsumption only works through the same named concepts.
- Printing exact halves such as 2.25 with one decimal rounds differently between C libraries, so tests use values that are not exact halves.

## Run it
```bash
./procpp.sh 65                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_65_concepts       # the interactive demo
ctest --test-dir build -R test_day_65 --output-on-failure
```

## Next step
- Day 66 replaces hand-written loops with ranges and views.
