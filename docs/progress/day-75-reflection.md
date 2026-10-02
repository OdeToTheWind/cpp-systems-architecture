# Day 75 – Coroutines Reflection

**Date:** 2026-05-27 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_75_coroutines/lesson.hpp`](../../src/day_75_coroutines/lesson.hpp) · **Tests:** [`tests/test_day_75.cpp`](../../tests/test_day_75.cpp) (7 tests)

## Scenario
The scripting layer of a *story-driven game*. Level designers want to write a cutscene top to bottom – "say this, wait until the player orders, say that" – instead of splitting it into callbacks. Coroutines make that possible: generators produce lazy sequences (dialogue lines, an endless ID stream) and awaitable scenes pause until a game event resumes them.

## Syllabus deliverables
> co\_yield generators, co\_await basics, promise types, lazy sequences

| Deliverable | Implemented in |
|---|---|
| ✅ a generator type with its promise\_type | `Generator` |
| ✅ an infinite lazy sequence with co\_yield | `entity_ids` |
| ✅ a lazy pipeline that reads only what it needs | `dialogue_lines` |
| ✅ an awaitable that suspends until an event | `Director::wait_for` |
| ✅ a scene coroutine written top to bottom | `tavern_scene` |

## Key learnings
- A function becomes a coroutine when it uses co_yield, co_await or co_return; its return type's promise_type controls how it behaves.
- initial_suspend returning suspend_always makes a coroutine lazy – nothing runs until it is resumed.
- A generator turns co_yield into a range-for: each ++ resumes the coroutine to its next yield, so infinite sequences are fine.
- An awaitable's await_suspend receives the paused coroutine's handle, so an event loop can resume it later.

## Pitfalls I hit (and how I fixed them)
- A generator that took its text by const reference read a destroyed temporary; coroutine parameters are now taken by value.
- Forgetting to destroy the coroutine handle leaked the frame; the owning types destroy it in their destructors.
- Resuming waiters while iterating the map broke when a scene waited again for the same event; the list is now moved out first.

## Run it
```bash
./procpp.sh 75                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_75_coroutines       # the interactive demo
ctest --test-dir build -R test_day_75 --output-on-failure
```

## Next step
- Day 76 goes below mutexes to atomics and memory ordering.
