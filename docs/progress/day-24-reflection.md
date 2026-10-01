# Day 24 – Debugging Techniques Reflection

**Date:** 2026-04-06 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_24_debugging/lesson.hpp`](../../src/day_24_debugging/lesson.hpp) · **Tests:** [`tests/test_day_24.cpp`](../../tests/test_day_24.cpp) (7 tests)

## Scenario
A *library late-fee calculator* that shipped with a real bug: some borrowers were charged for the grace day. The bug is reproduced with a minimal failing case, traced with debug output on std::cerr, cornered by bisecting the inputs, and fixed – with assertions guarding the invariants so it cannot come back.

## Syllabus deliverables
> Reproducing bugs, assertions, std::cerr tracing, bisecting failing inputs, debugger-friendly code

| Deliverable | Implemented in |
|---|---|
| ✅ the shipped bug, kept for study | `late_fee_buggy` |
| ✅ debugger-friendly code: one named step per line | `late_fee` |
| ✅ runtime contract checks that throw instead of aborting | `expects` |
| ✅ opt-in tracing to std::cerr (or any stream) | `Tracer` |
| ✅ bisecting to the first failing input | `first_failing_day` |
| ✅ a minimal reproduction report | `reproduce` |

## Key learnings
- The first step of every fix is a minimal reproduction: the exact input, the expected output and the actual output.
- Debug traces belong on `std::cerr` (or an injected stream) and behind a switch, so normal output stays clean and tests can capture them.
- When failures start at some unknown input, binary search finds the first failing case in about log2(n) checks.
- Contract checks that throw (`expects`) document invariants, run in every build and can themselves be tested; `assert` disappears when `NDEBUG` is set.

## Pitfalls I hit (and how I fixed them)
- The shipped fee used `>= grace_days`, charging people who returned on the last free day – the reproduction for day 3 showed it immediately.
- Printing `late_fee(...)` inside an output expression put the trace lines in the middle of the result line; the fee is now computed first.
- Computing the bisection midpoint as `(low + high) / 2` can overflow; `low + (high - low) / 2` cannot.

## Run it
```bash
./procpp.sh 24                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_24_debugging       # the interactive demo
ctest --test-dir build -R test_day_24 --output-on-failure
```

## Next step
- Day 25 sets up the development environment that makes all of this fast: presets, warnings and sanitizers.
