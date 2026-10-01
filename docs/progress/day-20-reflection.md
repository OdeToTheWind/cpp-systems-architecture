# Day 20 – Returning Functions Reflection

**Date:** 2026-04-02 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_20_returning_functions/lesson.hpp`](../../src/day_20_returning_functions/lesson.hpp) · **Tests:** [`tests/test_day_20.cpp`](../../tests/test_day_20.cpp) (8 tests)

## Scenario
A *shipping-rate engine for an online shop*. Carrier rates are plain functions looked up through function pointers, promotions are lambdas built at run time and composed into one pricing rule, quotes come back as structs, and the checkout page receives each quote through a callback.

## Syllabus deliverables
> Returning values and structs, function pointers, std::function, lambdas returned from functions, callbacks

| Deliverable | Implemented in |
|---|---|
| ✅ returning a struct with several values | `Quote` |
| ✅ a function pointer type and a lookup returning one | `rate_function_for` |
| ✅ a lambda returned from a function, capturing by value | `make_discount` |
| ✅ std::function composing two pricing rules | `compose` |
| ✅ returning std::optional when there may be no answer | `cheapest_quote` |
| ✅ a callback invoked for every result | `quote_all` |

## Key learnings
- A function pointer (`long long (*)(int)`) can store any free function with that exact signature, and a lookup table can return one at run time.
- A lambda returned from a function must capture by value – a reference capture would point at the factory's dead local variables.
- `std::function` can hold function pointers, lambdas and function objects alike, at the cost of a possible allocation and an indirect call.
- Callbacks invert control: the engine produces quotes, the caller decides what to do with each one.

## Pitfalls I hit (and how I fixed them)
- An unknown operator silently fell back to addition in the first engine; unknown carriers now return `nullptr` and are reported.
- `compose(f, g)` first applied `g` – the test that a discount followed by a fee differs from a fee followed by a discount caught it.
- Calling an empty `std::function` throws `std::bad_function_call`; `quote` checks `if (rule)` before applying an optional rule.

## Run it
```bash
./procpp.sh 20                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_20_returning_functions       # the interactive demo
ctest --test-dir build -R test_day_20 --output-on-failure
```

## Next step
- Day 21 asks whether a function should return its result or print it.
