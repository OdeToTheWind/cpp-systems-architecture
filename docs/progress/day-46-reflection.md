# Day 46 – Variadic Templates Reflection

**Date:** 2026-04-28 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_46_variadic_templates/lesson.hpp`](../../src/day_46_variadic_templates/lesson.hpp) · **Tests:** [`tests/test_day_46.cpp`](../../tests/test_day_46.cpp) (8 tests)

## Scenario
A *structured logging and metrics helper for a game server*. One `log` call takes any number of key/value pairs of any types, a `sum_all` adds whatever numbers it is given, tuples of coordinates are unpacked straight into function calls, and a factory forwards its arguments perfectly to whatever object it creates.

## Syllabus deliverables
> Parameter packs, fold expressions, perfect forwarding, std::tuple and std::apply

| Deliverable | Implemented in |
|---|---|
| ✅ a parameter pack and sizeof... | `count_args` |
| ✅ a fold expression over + | `sum_all` |
| ✅ a fold over the comma operator to print key/value pairs | `format_fields` |
| ✅ perfect forwarding with std::forward | `make_tracked` |
| ✅ std::apply unpacks a tuple into a call | `distance_from_origin` |
| ✅ returning several values as a tuple | `min_max_mean` |

## Key learnings
- A parameter pack (`typename... Args`) captures any number of template arguments, and `sizeof...` counts them at compile time.
- Fold expressions such as `(0 + ... + values)` expand a pack with an operator; an initial value makes the empty pack well-defined.
- `std::forward<Args>(args)...` passes each argument on with its original value category, so temporaries are moved and named objects copied.
- `std::apply` calls a function with the elements of a tuple as separate arguments – the reverse of packing them.

## Pitfalls I hit (and how I fixed them)
- A unary fold `(... + values)` failed to compile for an empty pack; the binary fold with `0` fixed it.
- Forwarding with `std::move` instead of `std::forward` moved from the caller's named string and left it empty.
- An empty field list left a counter set but never read, which `-Wunused-but-set-variable` flagged in that one instantiation.

## Run it
```bash
./procpp.sh 46                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_46_variadic_templates       # the interactive demo
ctest --test-dir build -R test_day_46 --output-on-failure
```

## Next step
- Day 47 structures a desktop GUI so its logic can be tested without a window.
