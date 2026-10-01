# Day 21 – Return vs Print Reflection

**Date:** 2026-04-03 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_21_return_vs_print/lesson.hpp`](../../src/day_21_return_vs_print/lesson.hpp) · **Tests:** [`tests/test_day_21.cpp`](../../tests/test_day_21.cpp) (7 tests)

## Scenario
An *apartment-building electricity billing tool*. The same tiered tariff is written twice: once as a function that prints as it calculates (hard to reuse or test), and once as pure functions that return a bill which is formatted separately – and only the second design can total the whole building.

## Syllabus deliverables
> Pure functions vs side effects, returning data and formatting it separately, testable design

| Deliverable | Implemented in |
|---|---|
| ✅ a print-only function with side effects (the anti-pattern) | `print_bill_badly` |
| ✅ a pure function: same input, same output, no I/O | `energy_charge_cents` |
| ✅ returning structured data instead of text | `compute_bill` |
| ✅ formatting kept separate from calculation | `format_bill` |
| ✅ reuse that only the returning design allows | `building_total` |
| ✅ what reusing printed output costs | `total_from_printed_bill` |

## Key learnings
- A pure function depends only on its arguments and has no side effects, so the same input always gives the same output and it can be tested with one line.
- Returning a struct keeps every number available to the caller; formatting it is a separate, replaceable step.
- A function that prints can only be tested by capturing and parsing its text, which breaks as soon as the wording changes.
- Separating calculation from presentation lets the same bill feed a receipt, a CSV export and a building total.

## Pitfalls I hit (and how I fixed them)
- The first version printed each flat's bill, so totalling the building meant parsing text – `building_total` now sums returned `Bill`s.
- Changing 'total' to 'amount due' in the printed bill broke the parser, which is exactly why the returning design exists.
- Rounding VAT in two places gave totals one cent apart; the VAT is now computed once in `compute_bill`.

## Run it
```bash
./procpp.sh 21                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_21_return_vs_print       # the interactive demo
ctest --test-dir build -R test_day_21 --output-on-failure
```

## Next step
- Day 22 documents these functions so others can use them without reading their bodies.
