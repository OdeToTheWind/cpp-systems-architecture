# Day 06 – Data Types & Fixed-width Integers Reflection

**Date:** 2026-03-19 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_06_data_types/lesson.hpp`](../../src/day_06_data_types/lesson.hpp) · **Tests:** [`tests/test_day_06.cpp`](../../tests/test_day_06.cpp) (7 tests)

## Scenario
A *weather-station firmware memory planner*. The microcontroller has a few kilobytes of RAM, so every sensor field must use the smallest integer type that can hold its range – and the team must know exactly when a counter will wrap around.

## Syllabus deliverables
> Fundamental types and modifiers, sizeof and numeric\_limits, fixed-width integers, signed vs unsigned wrap-around

| Deliverable | Implemented in |
|---|---|
| ✅ one row per fundamental type and modifier | `type_table` |
| ✅ sizeof and std::numeric\_limits for any type | `describe_type` |
| ✅ choosing the smallest fixed-width integer | `smallest_fitting_type` |
| ✅ unsigned arithmetic wraps modulo 2^N | `counter_after` |
| ✅ floating-point precision limits | `float_loses_integer` |
| ✅ a RAM budget built from fixed-width types | `memory_plan` |

## Key learnings
- The standard only guarantees minimum sizes and the ordering `short <= int <= long <= long long`; `std::int32_t` and friends give exact widths when they matter.
- `std::numeric_limits<T>` reports the range of any arithmetic type, and `lowest()` (not `min()`) is the most negative floating-point value.
- Unsigned arithmetic wraps modulo 2^N by definition, while signed overflow is undefined behaviour – counters that must wrap belong in unsigned types.
- A `float` has a 24-bit significand, so integers above 16,777,216 are no longer all representable.

## Pitfalls I hit (and how I fixed them)
- Printing `numeric_limits<char>::max()` printed a raw byte instead of 127 – unary `+` promotes character types to `int` before printing.
- Comparing a negative range bound with an unsigned maximum gave the wrong answer; `std::cmp_less_equal` compares signed and unsigned values safely.
- An 8-bit packet counter silently reset to 0 after 255 packets – the planner now shows exactly when a chosen type wraps.

## Run it
```bash
./procpp.sh 6                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_06_data_types       # the interactive demo
ctest --test-dir build -R test_day_06 --output-on-failure
```

## Next step
- Day 07 converts between these types on purpose, with casts that make every conversion visible.
