# Day 74 – Concurrency: Futures & Thread Pools Reflection

**Date:** 2026-05-26 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_74_futures_thread_pools/lesson.hpp`](../../src/day_74_futures_thread_pools/lesson.hpp) · **Tests:** [`tests/test_day_74.cpp`](../../tests/test_day_74.cpp) (7 tests)

## Scenario
The *thumbnail service* of a photo-sharing site. Every upload needs several thumbnails and a checksum; the work is independent per image, so it runs in parallel. Results and errors travel back through futures, and a fixed-size thread pool keeps a burst of uploads from starting hundreds of threads.

## Syllabus deliverables
> std::async, promises and futures, exception propagation, a fixed-size thread pool

| Deliverable | Implemented in |
|---|---|
| ✅ running independent work with std::async | `checksums_async` |
| ✅ handing a result from one thread to another with a promise | `fetch_metadata` |
| ✅ exceptions that travel through future::get | `make_thumbnail` |
| ✅ a fixed-size thread pool that returns futures | `ThreadPool::submit` |
| ✅ collecting successes and failures from many tasks | `process_uploads` |

## Key learnings
- std::async runs a function on another thread and returns a future; get() waits for the result.
- A promise is the writing end of a future: one thread sets a value or an exception, another receives it.
- Exceptions thrown inside a task are stored in its future and rethrown by get(), so errors are not lost between threads.
- A fixed-size thread pool with a task queue bounds the number of threads however many tasks arrive.

## Pitfalls I hit (and how I fixed them)
- Calling get() inside the loop that launched each std::async made the work sequential; the futures are now collected first.
- `std::function` cannot hold a move-only `packaged_task`, so the pool wraps it in a shared_ptr.
- Running a task while holding the queue mutex serialised the pool; tasks now run after the lock is released.

## Run it
```bash
./procpp.sh 74                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_74_futures_thread_pools       # the interactive demo
ctest --test-dir build -R test_day_74 --output-on-failure
```

## Next step
- Day 75 writes code that pauses and resumes itself: coroutines.
