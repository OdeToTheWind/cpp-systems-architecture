# Day 71 – Functional Tools Reflection

**Date:** 2026-05-23 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_71_functional/lesson.hpp`](../../src/day_71_functional/lesson.hpp) · **Tests:** [`tests/test_day_71.cpp`](../../tests/test_day_71.cpp) (7 tests)

## Scenario
The *pricing rules engine* of an online shop. Discounts, taxes and rounding are small functions that marketing can combine per campaign: rules are lambdas that capture their settings, pipelines are built by composing functions, member pointers are called through std::invoke, regional tax uses std::bind_front, and an expensive shipping-rate lookup is memoised.

## Syllabus deliverables
> Lambdas and captures, std::invoke, std::bind\_front, higher-order functions, memoisation

| Deliverable | Implemented in |
|---|---|
| ✅ lambdas that capture their settings by value | `percent_off` |
| ✅ an init-capture with mutable state | `make_ticket_counter` |
| ✅ composing functions into a pipeline | `compose` |
| ✅ summing any member through std::invoke | `total_of` |
| ✅ pre-binding arguments with std::bind\_front | `regional_tax` |
| ✅ caching the results of an expensive function | `Memoised` |

## Key learnings
- Lambdas capture by value (a snapshot) or by reference (a live view); returning a lambda requires value captures.
- Init-captures such as `[next = first]` with `mutable` give a lambda private state that changes between calls.
- std::invoke calls data-member pointers, member-function pointers and ordinary callables uniformly, which makes projections easy.
- Memoisation trades memory for time: a map from arguments to results makes repeated expensive calls free.

## Pitfalls I hit (and how I fixed them)
- A rule that captured a local by reference read a dead variable after the function returned.
- `std::bind_front` copies its bound arguments, so a later change to the tax table did not affect existing rules – intended here, but surprising at first.
- Memoising a function with side effects would skip them; only pure functions should be cached.

## Run it
```bash
./procpp.sh 71                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_71_functional       # the interactive demo
ctest --test-dir build -R test_day_71 --output-on-failure
```

## Next step
- Day 72 collects these tools into classic design patterns.
