# Day 28 – Creating Classes Reflection

**Date:** 2026-04-10 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_28_classes/lesson.hpp`](../../src/day_28_classes/lesson.hpp) · **Tests:** [`tests/test_day_28.cpp`](../../tests/test_day_28.cpp) (7 tests)

## Scenario
A *gym class booking system*. A `FitnessClass` object guards its own rules – never more attendees than places, nobody booked twice, the waiting list promoted in order – so no caller can ever put it into an invalid state.

## Syllabus deliverables
> Class definitions, constructors, member functions, invariants, explicit constructors, operator<<

| Deliverable | Implemented in |
|---|---|
| ✅ a small value class with an explicit constructor | `TimeSlot` |
| ✅ a class whose constructor establishes the invariant | `FitnessClass::FitnessClass` |
| ✅ member functions that preserve the invariant | `FitnessClass::book` |
| ✅ cancelling promotes the waiting list in order | `FitnessClass::cancel` |
| ✅ a self-check of the invariant | `FitnessClass::invariant_holds` |
| ✅ stream output with operator<< | `operator<<` |

## Key learnings
- A class invariant is a rule that is true for every object between public calls; the constructor establishes it and every member function preserves it.
- `explicit` on single-argument constructors stops surprising implicit conversions such as `TimeSlot slot = 1080;`.
- Overloading `operator<<` as a non-member that returns the stream makes objects printable and chainable with `<<`.
- `= default` comparison operators (C++20) compare members in order and need no hand-written code.

## Pitfalls I hit (and how I fixed them)
- Leaving the waiting list did not free a place, but the first version promoted someone anyway; `cancel` now distinguishes the two lists.
- `operator<<` changed the stream's fill character to `0` and left it that way; it now restores the previous fill.
- Public vectors let a caller push a twelfth attendee into an eleven-place class; the members are private and exposed as const references.

## Run it
```bash
./procpp.sh 28                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_28_classes       # the interactive demo
ctest --test-dir build -R test_day_28 --output-on-failure
```

## Next step
- Day 29 packages code as a library and uses it the way third-party libraries are used.
