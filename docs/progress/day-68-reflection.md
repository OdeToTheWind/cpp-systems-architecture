# Day 68 – Move Semantics Reflection

**Date:** 2026-05-20 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_68_move_semantics/lesson.hpp`](../../src/day_68_move_semantics/lesson.hpp) · **Tests:** [`tests/test_day_68.cpp`](../../tests/test_day_68.cpp) (7 tests)

## Scenario
The sample buffers of a *podcast audio editor*. An hour of stereo audio is hundreds of megabytes, so whether a buffer is copied or moved decides whether an edit is instant or slow. The buffer manages its own memory with the rule of five and counts every allocation, copy and move, so each claim about move semantics can be checked by a test.

## Syllabus deliverables
> Lvalues and rvalues, move constructors and assignment, the rule of five, std::move vs copy

| Deliverable | Implemented in |
|---|---|
| ✅ telling lvalues from rvalues with overloads | `category` |
| ✅ a buffer that follows the rule of five | `AudioBuffer` |
| ✅ a sink parameter taken by value and moved in | `Track::set_samples` |
| ✅ returning large objects by value | `mixdown` |
| ✅ noexcept moves letting a vector move instead of copy | `grow_library` |

## Key learnings
- An lvalue has a name and an address; an rvalue is a temporary. `std::move` does not move anything – it casts to an rvalue so a move overload can be chosen.
- A move constructor steals the source's resources and leaves it valid but empty; it should be noexcept.
- The rule of five: a class that manages a resource needs a destructor, copy and move constructors, and copy and move assignment.
- Sink parameters taken by value and moved into place cost one copy for lvalues and none for rvalues.

## Pitfalls I hit (and how I fixed them)
- Without `noexcept` on the move constructor, `std::vector` copied every buffer when it grew, to keep its strong exception guarantee.
- `std::move` on a const object quietly called the copy constructor.
- Using a buffer after moving from it read an empty buffer; moved-from objects should only be assigned to or destroyed.

## Run it
```bash
./procpp.sh 68                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_68_move_semantics       # the interactive demo
ctest --test-dir build -R test_day_68 --output-on-failure
```

## Next step
- Day 69 makes types feel built-in with operator overloading.
