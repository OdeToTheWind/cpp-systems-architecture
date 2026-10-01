# Day 31 – Member Functions Reflection

**Date:** 2026-04-13 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_31_member_functions/lesson.hpp`](../../src/day_31_member_functions/lesson.hpp) · **Tests:** [`tests/test_day_31.cpp`](../../tests/test_day_31.cpp) (8 tests)

## Scenario
A *coffee-roastery batch tracker*. Each roast batch has methods that change it (and return `*this` so they chain), const methods that only read it, and static members that belong to the roastery as a whole: the batch-number counter and the list of beans it buys.

## Syllabus deliverables
> Const member functions, static member functions, static data, this, method chaining

| Deliverable | Implemented in |
|---|---|
| ✅ static data shared by every object | `RoastBatch::next_number_` |
| ✅ a static member function that needs no object | `RoastBatch::is_known_bean` |
| ✅ a static factory function | `RoastBatch::parse` |
| ✅ chaining: mutators return \*this | `RoastBatch::add_beans` |
| ✅ const member functions that only read | `RoastBatch::weight_loss_percent` |
| ✅ using this to compare with another object | `RoastBatch::same_origin_as` |

## Key learnings
- Static data members belong to the class, not to an object – one batch counter is shared by every `RoastBatch`.
- Static member functions have no `this`, so they can answer class-wide questions (`is_known_bean`) or act as named factories (`parse`).
- A mutator that returns `*this` by reference allows chaining: `batch.add_beans(1000).set_roast(Roast::dark)`.
- `inline static` data members (C++17) can be defined inside the class, without a separate definition in a .cpp file.

## Pitfalls I hit (and how I fixed them)
- A setter returned `RoastBatch` by value, so the chained call modified a temporary copy; it now returns `RoastBatch&`.
- Calling `weight_loss_percent()` on a const batch failed to compile until the function was marked `const`.
- `same_origin_as(*this)` reported a batch as a duplicate of itself; comparing `this` with `&other` excludes it.

## Run it
```bash
./procpp.sh 31                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_31_member_functions       # the interactive demo
ctest --test-dir build -R test_day_31 --output-on-failure
```

## Next step
- Day 32 studies the constructors that create these objects.
