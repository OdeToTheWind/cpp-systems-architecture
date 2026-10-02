# Day 73 – Concurrency: Threads & Mutexes Reflection

**Date:** 2026-05-25 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_73_threads/lesson.hpp`](../../src/day_73_threads/lesson.hpp) · **Tests:** [`tests/test_day_73.cpp`](../../tests/test_day_73.cpp) (7 tests)

## Scenario
A *print shop's job server*. Several front desks submit print jobs into a bounded queue, several printers take jobs from it, and the shop counts pages per printer. The queue blocks producers when it is full and consumers when it is empty – a classic producer-consumer design built from std::thread, std::mutex and std::condition_variable.

## Syllabus deliverables
> std::thread, mutex and lock\_guard, condition variables, producer-consumer queues

| Deliverable | Implemented in |
|---|---|
| ✅ splitting work across std::thread and merging under a lock\_guard | `parallel_word_count` |
| ✅ locking two mutexes without deadlock | `transfer` |
| ✅ a bounded queue whose push waits on a condition variable | `BoundedQueue::push` |
| ✅ a pop that ends cleanly when the queue is closed | `BoundedQueue::pop` |
| ✅ producers and consumers sharing the queue | `run_print_shop` |

## Key learnings
- std::thread starts running at once and must be joined (or detached) before it is destroyed, or the program terminates.
- Shared data needs a mutex; lock_guard and scoped_lock unlock automatically, even when an exception is thrown.
- A condition variable lets a thread sleep until another thread changes the state; the wait predicate handles spurious wake-ups.
- Giving each thread private data and merging once at the end avoids most locking.

## Pitfalls I hit (and how I fixed them)
- Two transfers in opposite directions locked their mutexes in opposite orders and deadlocked; std::scoped_lock locks both safely.
- Consumers waited forever after the producers finished until the queue gained close() and notify_all().
- Tests that expected a particular thread order were flaky; tests now check totals and invariants that hold for every interleaving.

## Run it
```bash
./procpp.sh 73                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_73_threads       # the interactive demo
ctest --test-dir build -R test_day_73 --output-on-failure
```

## Next step
- Day 74 gets results back from threads with futures and a thread pool.
