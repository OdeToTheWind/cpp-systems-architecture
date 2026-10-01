# Day 05 – Mathematical Operations Reflection

**Date:** 2026-03-18 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_05_math_operations/lesson.hpp`](../../src/day_05_math_operations/lesson.hpp) · **Tests:** [`tests/test_day_05.cpp`](../../tests/test_day_05.cpp) (10 tests)

## Scenario
A *restaurant bill splitter* that works in whole cents: it applies a discount before tax, splits the total fairly between diners, and refuses calculations that would overflow or divide by zero.

## Syllabus deliverables
> Arithmetic and compound assignment, precedence, integer vs floating division, overflow-safe arithmetic, cmath functions

| Deliverable | Implemented in |
|---|---|
| ✅ compound assignment (-=, \*=, /=, %=) on money | `apply_discount_and_tax` |
| ✅ operator precedence and associativity | `precedence_examples` |
| ✅ integer division truncates, floor division rounds down | `division_facts` |
| ✅ overflow-safe multiplication | `checked_multiply` |
| ✅ distributing a remainder fairly | `split_bill` |
| ✅ cmath: pow, sqrt, hypot and std::numbers::pi | `circle_stats` |

## Key learnings
- C++ integer division truncates toward zero (`-7 / 2 == -3`) and `%` takes the sign of the dividend, so `a == b * (a / b) + a % b` always holds.
- Signed overflow is undefined behaviour – the check must happen *before* the multiplication, by comparing against `numeric_limits<long long>::max() / b`.
- Money is safest as whole cents in an integer type; rounding is then an explicit step (`+ 50` then `/ 100` rounds half up).
- `std::numbers::pi` (C++20) replaces hand-typed constants, and `std::hypot` avoids overflow when squaring large sides.

## Pitfalls I hit (and how I fixed them)
- The first bill calculator *added* the discount to the price (`price += discount`); the discount is now subtracted with `-=` before tax is applied.
- `LLONG_MIN / -1` overflows even though it looks harmless – `division_facts` now rejects that single case explicitly.
- Splitting 100.00 three ways lost a cent with `double`; `split_bill` hands the leftover cents to the first diners so the shares always add up.

## Run it
```bash
./procpp.sh 5                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_05_math_operations       # the interactive demo
ctest --test-dir build -R test_day_05 --output-on-failure
```

## Next step
- Day 06 looks at how many bytes each of these number types really has, and what happens at their limits.
