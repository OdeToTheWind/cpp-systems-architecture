# Day 12 – Functions Reflection

**Date:** 2026-03-25 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_12_functions/lesson.hpp`](../../src/day_12_functions/lesson.hpp) · **Tests:** [`tests/test_day_12.cpp`](../../tests/test_day_12.cpp) (7 tests)

## Scenario
A *bakery order counter*. Small, documented functions price each pastry by size, build up an order, apply a loyalty-card discount and print the receipt. This header holds only the declarations; their definitions live in lesson.cpp.

## Syllabus deliverables
> Declarations vs definitions, parameters and return values, pass by value and const reference, overloading, default arguments

| Deliverable | Implemented in |
|---|---|
| ✅ a declaration with a default argument (definition in lesson.cpp) | `unit_price` |
| ✅ pass by reference to modify the caller's object | `add_item` |
| ✅ pass by const reference to read without copying | `order_total` |
| ✅ pass by value for a cheap, independent copy | `apply_loyalty` |
| ✅ overloading on the parameter list | `format_price` |

## Key learnings
- A declaration tells the compiler a function exists; the single definition provides its body – the header declares, `lesson.cpp` defines.
- Default arguments belong in the declaration only; repeating them in the definition is a compile error.
- Pass large objects by `const&` to read them without copying, by `&` when the function must modify the caller's object, and by value when the function needs its own copy.
- Overloads share a name but differ in parameter lists; delegating from one overload to another keeps a single formatting rule.

## Pitfalls I hit (and how I fixed them)
- Writing the default argument in both the header and the definition caused 'redefinition of default argument' – it now lives only in the header.
- `add_item(Order order, …)` added items to a copy that vanished on return; the parameter is now `Order&`.
- Helper functions defined in the header caused multiple-definition link errors once two files included it; helpers now live in an anonymous namespace in `lesson.cpp`.

## Run it
```bash
./procpp.sh 12                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_12_functions       # the interactive demo
ctest --test-dir build -R test_day_12 --output-on-failure
```

## Next step
- Day 13 repeats work with for loops over the data these functions produce.
