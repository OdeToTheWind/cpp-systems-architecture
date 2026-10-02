# Day 76 – Atomics & Memory Order Reflection

**Date:** 2026-05-28 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_76_atomics/lesson.hpp`](../../src/day_76_atomics/lesson.hpp) · **Tests:** [`tests/test_day_76.cpp`](../../tests/test_day_76.cpp) (7 tests)

## Scenario
The *live metrics* of a busy web server. Every request thread bumps counters and records its latency; a dashboard thread reads them. Taking a mutex on every request would make the metrics the bottleneck, so they use atomics: relaxed counters, a compare-exchange loop for the slowest request, a spin lock, and a release/acquire handshake that publishes a config snapshot safely.

## Syllabus deliverables
> std::atomic, compare-exchange, memory ordering, lock-free counters and flags

| Deliverable | Implemented in |
|---|---|
| ✅ a lock-free counter with relaxed ordering | `Metrics::record` |
| ✅ a compare-exchange loop that tracks a maximum | `update_max` |
| ✅ a spin lock built from atomic\_flag with acquire and release | `SpinLock` |
| ✅ publishing data with a release store and an acquire load | `Mailbox::publish` |
| ✅ running request threads against shared metrics | `simulate_traffic` |

## Key learnings
- std::atomic makes single operations indivisible; fetch_add on a counter needs no mutex.
- compare_exchange_weak in a loop implements any read-modify-write, such as "raise to the maximum".
- memory_order_relaxed is enough for independent counters; a release store paired with an acquire load also publishes the data written before it.
- atomic_flag with acquire/release is enough to build a spin lock that works with lock_guard.

## Pitfalls I hit (and how I fixed them)
- Publishing a config with a relaxed flag store let a reader see the flag before the data; the store is now release and the load acquire.
- compare_exchange_weak can fail spuriously, so it is always used in a loop.
- A spin lock burned a whole core under contention; it now yields while waiting and is used only for tiny critical sections.

## Run it
```bash
./procpp.sh 76                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_76_atomics       # the interactive demo
ctest --test-dir build -R test_day_76 --output-on-failure
```

## Next step
- Day 77 adds structured logging and configuration.
