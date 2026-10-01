# Day 40 – Iterators Reflection

**Date:** 2026-04-22 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_40_iterators/lesson.hpp`](../../src/day_40_iterators/lesson.hpp) · **Tests:** [`tests/test_day_40.cpp`](../../tests/test_day_40.cpp) (7 tests)

## Scenario
A *radio station's "recently played" board*. The last N tracks live in a fixed-size ring buffer with its own iterator, so the board works with range-based for and with every standard algorithm; the station's library is cleaned with erase loops that never use an invalidated iterator.

## Syllabus deliverables
> Iterator categories, begin and end, writing a custom iterator, iterator invalidation

| Deliverable | Implemented in |
|---|---|
| ✅ naming the iterator category of any container | `category_of` |
| ✅ a container with begin() and end() | `RingBuffer` |
| ✅ a custom forward iterator | `RingBuffer::Iterator` |
| ✅ erasing while iterating without invalid iterators | `remove_short_tracks` |
| ✅ algorithms working through iterators | `longest_recent_track` |

## Key learnings
- Iterator categories form a hierarchy – input, forward, bidirectional, random access, contiguous – and algorithms require a minimum category.
- Any type with `begin()` and `end()` returning iterators works in a range-based for loop and with the standard algorithms.
- A custom iterator needs the member types, `*`, `++` and `==`; C++20's `std::forward_iterator` concept checks the requirements at compile time.
- `erase` invalidates the erased iterator, so loops must continue from the iterator `erase` returns; C++20 `std::erase_if` avoids the loop entirely.

## Pitfalls I hit (and how I fixed them)
- `for (it = list.begin(); …; ++it) if (short) list.erase(it);` incremented an invalidated iterator and crashed; the loop now uses `it = list.erase(it)`.
- A comma inside `RingBuffer<int, 2>` split a test macro's arguments; wrapping the argument in parentheses fixed it.
- The first ring buffer iterated from index 0 instead of the oldest element; the iterator now counts logical positions from `start_`.

## Run it
```bash
./procpp.sh 40                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_40_iterators       # the interactive demo
ctest --test-dir build -R test_day_40 --output-on-failure
```

## Next step
- Day 41 reads and writes files with fstream.
