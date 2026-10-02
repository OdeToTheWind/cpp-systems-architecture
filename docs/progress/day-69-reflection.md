# Day 69 – Operator Overloading Reflection

**Date:** 2026-05-21 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_69_operator_overloading/lesson.hpp`](../../src/day_69_operator_overloading/lesson.hpp) · **Tests:** [`tests/test_day_69.cpp`](../../tests/test_day_69.cpp) (7 tests)

## Scenario
A *recipe scaler* for a bakery. Quantities such as "1 1/2 cups" or "3/4 tsp" must be scaled by 2/3 or 5/2 without rounding errors, so they are stored as exact fractions. With overloaded operators the fraction type reads like a built-in number: arithmetic, comparisons with the spaceship operator, and reading and writing with streams.

## Syllabus deliverables
> Arithmetic and comparison operators, the spaceship operator, stream operators, invariants

| Deliverable | Implemented in |
|---|---|
| ✅ a fraction type whose invariant every operation keeps | `Rational` |
| ✅ compound assignment as the basis for binary operators | `Rational::operator+=` |
| ✅ three-way comparison by cross-multiplication | `operator<=>` |
| ✅ writing mixed numbers to a stream | `operator<<` |
| ✅ reading "1 1/2" or "3/4" from a stream | `operator>>` |

## Key learnings
- Operators should do what users expect: implement `+=` as a member and build `+` on top of it as a non-member, so `2 * r` and `r * 2` both work.
- A class invariant (reduced fraction, positive denominator) checked in the constructor lets `==` be defaulted.
- One `operator<=>` returning `std::strong_ordering` gives all of <, <=, > and >=.
- Stream operators should behave like the built-in ones: on bad input set failbit and leave the target unchanged.

## Pitfalls I hit (and how I fixed them)
- Without normalisation 1/2 and 2/4 compared unequal under a defaulted `==`.
- Reading "2 cups" swallowed "cups" while looking for a mixed number; the stream is now rolled back with seekg.
- An implicit conversion to double silently lost exactness, so the conversion is explicit.

## Run it
```bash
./procpp.sh 69                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_69_operator_overloading       # the interactive demo
ctest --test-dir build -R test_day_69 --output-on-failure
```

## Next step
- Day 70 moves work from run time to compile time.
