# Day 08 – Conditional Statements Reflection

**Date:** 2026-03-21 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_08_conditionals/lesson.hpp`](../../src/day_08_conditionals/lesson.hpp) · **Tests:** [`tests/test_day_08.cpp`](../../tests/test_day_08.cpp) (7 tests)

## Scenario
A *ski-resort lift operations board*. Every morning the duty manager enters the wind speed, temperature, visibility and fresh snow; the board decides which lifts may open, explains every closure, and rejects sensor readings that cannot be real.

## Syllabus deliverables
> if / else if / else chains, switch with enums, guard clauses, the conditional operator

| Deliverable | Implemented in |
|---|---|
| ✅ guard clauses reject impossible readings early | `validate` |
| ✅ an if / else if / else chain over ranges | `classify_wind` |
| ✅ switch over an enum class with no default | `lift_decision` |
| ✅ nested conditions kept shallow | `avalanche_warning` |
| ✅ the conditional operator for short choices | `status_label` |

## Key learnings
- In an `if / else if` chain each branch may assume every earlier condition was false, so ranges only need their upper bound.
- Guard clauses that return early on invalid input keep the main logic flat instead of nesting it three levels deep.
- A `switch` over an `enum class` without a `default` label lets `-Wswitch` warn when a new enumerator is added but not handled.
- The conditional operator is ideal for choosing between two short values such as labels; longer logic reads better as `if`.

## Pitfalls I hit (and how I fixed them)
- A wind speed of exactly 20 km/h fell into no category because two branches used `<` and `>`; half-open ranges (`< 20`, then `< 45`) leave no gaps.
- `NaN` from a broken sensor slipped through every comparison (all are false); `validate` now checks `std::isfinite` first.
- Adding a default label to the lift switch hid a forgotten lift type – removing it brought the compiler warning back.

## Run it
```bash
./procpp.sh 8                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_08_conditionals       # the interactive demo
ctest --test-dir build -R test_day_08 --output-on-failure
```

## Next step
- Day 09 combines several conditions with &&, || and ! and shows when C++ stops evaluating them.
