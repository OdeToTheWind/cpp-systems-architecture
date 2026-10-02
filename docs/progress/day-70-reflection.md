# Day 70 – Compile-time Programming Reflection

**Date:** 2026-05-22 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_70_compile_time/lesson.hpp`](../../src/day_70_compile_time/lesson.hpp) · **Tests:** [`tests/test_day_70.cpp`](../../tests/test_day_70.cpp) (7 tests)

## Scenario
The telemetry firmware of a *soil-moisture sensor board*. Packets are sent over a slow radio link, so the CRC lookup table is computed by the compiler instead of at boot, configuration constants are validated at compile time, and one serialise function handles every field type, choosing its encoding with type traits and `if constexpr`.

## Syllabus deliverables
> constexpr and consteval functions, static\_assert, type traits, if constexpr

| Deliverable | Implemented in |
|---|---|
| ✅ a lookup table built by the compiler | `crc8_table` |
| ✅ a constexpr checksum usable at compile time and run time | `crc8` |
| ✅ a consteval check that rejects bad constants at compile time | `checked_baud` |
| ✅ a custom type trait | `is_fixed_size` |
| ✅ one serialiser that branches on types with if constexpr | `serialise` |

## Key learnings
- constexpr functions can run at compile time or run time; a constexpr variable forces compile time, as with the CRC table.
- consteval functions must run at compile time, so an invalid constant becomes a build error.
- static_assert documents assumptions – packet sizes, check values – and fails the build when they stop holding.
- `if constexpr` discards the branches that do not apply, so one template can call `.size()` for strings and shifts for integers.

## Pitfalls I hit (and how I fixed them)
- A plain `if` instead of `if constexpr` failed to compile, because every branch was instantiated for every type.
- Shifting a negative signed integer is easy to get wrong; values are converted to the unsigned type first.
- Truncating 21.456 to centi-units gave 2145; the float is now rounded to 2146.

## Run it
```bash
./procpp.sh 70                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_70_compile_time       # the interactive demo
ctest --test-dir build -R test_day_70 --output-on-failure
```

## Next step
- Day 71 treats functions as values.
