# Day 48 – Static vs Dynamic Typing Reflection

**Date:** 2026-04-30 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_48_static_dynamic_typing/lesson.hpp`](../../src/day_48_static_dynamic_typing/lesson.hpp) · **Tests:** [`tests/test_day_48.cpp`](../../tests/test_day_48.cpp) (8 tests)

## Scenario
A *spreadsheet cell engine*. A cell may hold nothing, a number, text, a boolean or an error – decided at run time from what the user typed – yet every operation on cells is checked at compile time through std::variant and std::visit. Units are given their own types so metres and feet can never be added by accident, and a plugin metadata bag shows where std::any fits.

## Syllabus deliverables
> Static type checking, std::variant and std::visit, std::any, type-safe heterogeneous data

| Deliverable | Implemented in |
|---|---|
| ✅ strong types that the compiler keeps apart | `Length` |
| ✅ a closed set of run-time types: std::variant | `Cell` |
| ✅ deciding the type from text at run time | `parse_cell` |
| ✅ std::visit with an overload set | `display` |
| ✅ type-safe aggregation over mixed cells | `sum_numbers` |
| ✅ an open set of types: std::any and any\_cast | `PluginInfo` |

## Key learnings
- C++ is statically typed: giving each unit its own type (`Length<Metres>`, `Length<Feet>`) turns unit mix-ups into compile errors.
- `std::variant` holds exactly one of a fixed list of types, chosen at run time, while every operation on it is still checked at compile time.
- `std::visit` with an overload set must handle every alternative, so adding a new cell type forces every visitor to be updated.
- `std::any` can hold any type at all, but the type is only checked when reading with `any_cast` – right for plugin metadata, wrong for core data.

## Pitfalls I hit (and how I fixed them)
- Storing cells as strings and converting on every use made `"12 apples"` count as 12; the parser now decides the type once and keeps it.
- `std::get<double>` on a text cell threw `bad_variant_access` in the sum; `std::get_if` checks the alternative first.
- `any_cast<long>` on an `int` value threw – `std::any` requires the exact stored type, not a convertible one.

## Run it
```bash
./procpp.sh 48                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_48_static_dynamic_typing       # the interactive demo
ctest --test-dir build -R test_day_48 --output-on-failure
```

## Next step
- Day 49 designs error handling for a whole subsystem: exception hierarchies and error codes.
