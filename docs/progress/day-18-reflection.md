# Day 18 – Positional and Named Arguments Reflection

**Date:** 2026-03-31 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_18_named_arguments/lesson.hpp`](../../src/day_18_named_arguments/lesson.hpp) · **Tests:** [`tests/test_day_18.cpp`](../../tests/test_day_18.cpp) (7 tests)

## Scenario
An *airline booking API*. The route is passed positionally (it is always needed), optional extras have defaults, overloads accept a date in two shapes, and the many optional settings travel in a parameter struct filled with C++20 designated initialisers – C++'s closest equivalent to keyword arguments.

## Syllabus deliverables
> Positional parameters, default arguments, overloads, parameter structs and designated initialisers

| Deliverable | Implemented in |
|---|---|
| ✅ positional parameters for required data | `base_fare_cents` |
| ✅ default arguments for optional trailing parameters | `seat_fee_cents` |
| ✅ overloads accepting different argument shapes | `make_date` |
| ✅ a parameter struct used with designated initialisers | `BookingOptions` |
| ✅ a function taking the parameter struct | `book_flight` |
| ✅ a fluent builder as the named-parameter idiom | `BookingBuilder` |

## Key learnings
- C++ has only positional arguments; defaults can be omitted only from the right, so the order of parameters is part of the design.
- A struct of options with default member initialisers, filled with C++20 designated initialisers (`{.cabin = Cabin::premium, .bags = 2}`), reads like keyword arguments.
- The classic Named Parameter Idiom chains setters that return `*this`, which also works before C++20.
- Overloads let one operation accept different shapes of input, and one overload can delegate to another to share validation.

## Pitfalls I hit (and how I fixed them)
- A width of `0.0` was used to mean 'not given', so a real zero-width input silently became a square – an explicit overload or `std::optional` says what it means.
- Designated initialisers must follow the declaration order of the members; writing `.bags` before `.cabin` did not compile.
- `-Wmissing-field-initializers` fired for designated initialisers until every option had a default member initialiser.

## Run it
```bash
./procpp.sh 18                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_18_named_arguments       # the interactive demo
ctest --test-dir build -R test_day_18 --output-on-failure
```

## Next step
- Day 19 looks at what happens when functions receive pointers and references instead of copies.
