# Day 64 – Templates & Generic Programming Reflection

**Date:** 2026-05-16 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_64_templates/lesson.hpp`](../../src/day_64_templates/lesson.hpp) · **Tests:** [`tests/test_day_64.cpp`](../../tests/test_day_64.cpp) (7 tests)

## Scenario
The firmware library of a *weather station*. Temperature, wind-speed and status sensors all keep their recent readings, and all need the same "latest N values, min/max/mean" logic. Instead of three copies, one set of templates works for every value type, with specialisations where a type needs different treatment.

## Syllabus deliverables
> Function and class templates, template argument deduction, specialisation, dependent names

| Deliverable | Implemented in |
|---|---|
| ✅ a function template with argument deduction | `largest` |
| ✅ a class template with a non-type parameter | `RingBuffer` |
| ✅ class template argument deduction with a guide | `Reading` |
| ✅ full and partial specialisation | `TypeName` |
| ✅ dependent names with typename | `summarise` |

## Key learnings
- A function template is a recipe: the compiler deduces T from the arguments and generates one function per type actually used.
- Class templates can take values as parameters too – `RingBuffer<double, 5>` fixes its capacity at compile time with no heap allocation.
- Specialisation gives one type its own implementation; partial specialisation does it for a whole family such as every `std::vector<T>`.
- Inside a template, `Container::value_type` is a dependent name and needs `typename` to be read as a type.

## Pitfalls I hit (and how I fixed them)
- `add(1, 2.5)` returned int in a first version with one template parameter; two parameters and `decltype(a + b)` fixed it.
- Leaving the primary `TypeName` template undefined is deliberate: an unsupported type fails to compile instead of printing a wrong name.
- A ring buffer with capacity 0 divided by zero; a static_assert now rejects it at compile time.

## Run it
```bash
./procpp.sh 64                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_64_templates       # the interactive demo
ctest --test-dir build -R test_day_64 --output-on-failure
```

## Next step
- Day 65 states template requirements explicitly with concepts.
