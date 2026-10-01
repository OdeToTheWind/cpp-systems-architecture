# Day 17 – Vectors and Maps Reflection

**Date:** 2026-03-30 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_17_vectors_maps/lesson.hpp`](../../src/day_17_vectors_maps/lesson.hpp) · **Tests:** [`tests/test_day_17.cpp`](../../tests/test_day_17.cpp) (9 tests)

## Scenario
A *community tool library*. A `std::map` keeps the stock of every tool by name, a `std::vector` keeps the waiting list in arrival order, and borrow counts are sorted into a "most popular tools" board.

## Syllabus deliverables
> std::vector and std::map insert, lookup, update and erase, iteration, choosing the right container

| Deliverable | Implemented in |
|---|---|
| ✅ map insert-or-update with operator[] | `Inventory::add` |
| ✅ map lookup with find() that never inserts | `Inventory::quantity` |
| ✅ map erase by key | `Inventory::remove` |
| ✅ iterating a map in key order | `Inventory::report` |
| ✅ vector push\_back, erase and order | `Waitlist` |
| ✅ copying a map into a vector to sort by value | `most_borrowed` |
| ✅ the operator[] lookup pitfall | `brackets_insert_missing_keys` |

## Key learnings
- `std::map` keeps keys sorted and finds them in O(log n); `std::vector` keeps insertion order and gives O(1) access by index.
- `map[key]` inserts a value-initialised element when the key is missing, so read-only lookups must use `find()` or `contains()`.
- To rank a map by value, copy its pairs into a vector and sort that – a map can only be ordered by its key.
- C++20 `std::erase(vector, value)` replaces the error-prone erase-remove idiom.

## Pitfalls I hit (and how I fixed them)
- Checking stock with `stock["ladder"]` created a ladder with zero items in the report – `quantity` now uses `find`.
- An extra `std::cin.ignore()` swallowed the first letter of the first item (`milk` became `ilk`); reading whole lines with `std::getline` removed the need for it.
- Ties in the popularity board came out in a different order on another compiler until `std::sort` was replaced with `std::stable_sort`.

## Run it
```bash
./procpp.sh 17                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_17_vectors_maps       # the interactive demo
ctest --test-dir build -R test_day_17 --output-on-failure
```

## Next step
- Day 18 passes these containers to functions with clear, named options.
