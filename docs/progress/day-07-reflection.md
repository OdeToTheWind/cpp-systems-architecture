# Day 07 – Type Conversion & Casting Reflection

**Date:** 2026-03-20 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_07_type_conversion/lesson.hpp`](../../src/day_07_type_conversion/lesson.hpp) · **Tests:** [`tests/test_day_07.cpp`](../../tests/test_day_07.cpp) (7 tests)

## Scenario
A *payment-terminal message decoder*. Amounts arrive as raw integers and bytes from a card terminal; the decoder must convert them without silent truncation, talk to a legacy C checksum API, inspect the wire bytes, and tell payment messages from refunds.

## Syllabus deliverables
> Implicit promotions, static\_cast, const\_cast, reinterpret\_cast, dynamic\_cast, narrowing checks with brace initialisation

| Deliverable | Implemented in |
|---|---|
| ✅ integer promotion: uint8\_t + uint8\_t is an int | `promotion_report` |
| ✅ static\_cast to avoid integer division | `average_ticket` |
| ✅ checked narrowing that refuses lossy conversions | `narrow` |
| ✅ const\_cast to call a non-const legacy C API | `checksum_of` |
| ✅ reinterpret\_cast to inspect object bytes | `wire_bytes` |
| ✅ dynamic\_cast to recognise a derived message | `describe` |

## Key learnings
- Operands smaller than `int` are promoted to `int` before arithmetic, so `uint8_t(200) + uint8_t(100)` is the `int` 300, not a wrapped byte.
- Comparing a negative `int` with an `unsigned` converts the `int` to a huge unsigned value; `std::cmp_less` compares by value instead.
- `static_cast` is for well-defined conversions, `const_cast` only removes constness (writing through it to a truly const object is undefined), `reinterpret_cast` reinterprets bits, and `dynamic_cast` checks the real type of a polymorphic object at run time.
- A checked `narrow<To>(value)` that round-trips the value and compares signs turns silent truncation into a reported error.

## Pitfalls I hit (and how I fixed them)
- `total / count` truncated the average ticket to whole cents – `static_cast<double>` on one operand makes the division floating-point.
- `dynamic_cast` failed to compile because `Message` had no virtual function; a virtual destructor makes the class polymorphic and is required anyway.
- `narrow<unsigned>(-1)` passed the round-trip test (it comes back as -1), so the sign comparison was added to catch it.

## Run it
```bash
./procpp.sh 7                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_07_type_conversion       # the interactive demo
ctest --test-dir build -R test_day_07 --output-on-failure
```

## Next step
- Day 08 uses these values to make decisions with if, else and switch.
