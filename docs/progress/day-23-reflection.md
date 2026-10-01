# Day 23 – Scope, Lifetime & Global Variables Reflection

**Date:** 2026-04-05 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_23_scope_lifetime/lesson.hpp`](../../src/day_23_scope_lifetime/lesson.hpp) · **Tests:** [`tests/test_day_23.cpp`](../../tests/test_day_23.cpp) (7 tests)

## Scenario
A *deli-counter ticket dispenser*. Shop-wide settings live in a namespace, the ticket counter is a function-local static that survives between calls, helper code is hidden with internal linkage, and a lifetime log shows exactly when each object is born and destroyed.

## Syllabus deliverables
> Block, function, namespace and static scope, shadowing, object lifetime, internal linkage

| Deliverable | Implemented in |
|---|---|
| ✅ namespace-scope constants instead of mutable globals | `shop` |
| ✅ a function-local static that keeps its value between calls | `next_ticket` |
| ✅ internal linkage hides helpers in lesson.cpp | `format_ticket` |
| ✅ shadowing: an inner name hides an outer one | `shadowing_demo` |
| ✅ automatic, static and dynamic lifetimes side by side | `lifetime_demo` |

## Key learnings
- A name declared in a block is visible until the closing brace; a function-local `static` is initialised once, on first use, and lives until the program exits.
- An anonymous namespace gives names internal linkage, so helpers in one `.cpp` file can never clash with names in another.
- Automatic objects die at the end of their scope in reverse order, heap objects die when their owner releases them, and statics die after `main` returns.
- Constants in a namespace give the benefits of globals without the danger, because nobody can modify them.

## Pitfalls I hit (and how I fixed them)
- An inner `waiting` variable hid the outer one and the outer count never changed – `-Wshadow` reports this, and the demo silences it only around the deliberate example.
- The static `Tracked` object wrote into a log vector that had already been destroyed at exit; the log is now a static that is constructed first and destroyed last.
- A mutable global ticket counter could be reset from any file; it now lives behind `next_ticket()` with internal linkage.

## Run it
```bash
./procpp.sh 23                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_23_scope_lifetime       # the interactive demo
ctest --test-dir build -R test_day_23 --output-on-failure
```

## Next step
- Day 24 hunts down a real bug with tracing, assertions and bisection.
