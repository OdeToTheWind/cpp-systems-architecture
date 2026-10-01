# Day 50 – Exception Safety & RAII Reflection

**Date:** 2026-05-02 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_50_exception_safety/lesson.hpp`](../../src/day_50_exception_safety/lesson.hpp) · **Tests:** [`tests/test_day_50.cpp`](../../tests/test_day_50.cpp) (8 tests)

## Scenario
A *credit-union ledger*. A transfer touches two accounts and an audit journal; if anything throws halfway – a limit check, a full journal, a failing audit write – the ledger must never lose or create money. The lesson compares an unsafe transfer with versions that give the basic and the strong exception guarantee.

## Syllabus deliverables
> Basic, strong and nothrow guarantees, scope guards, copy-and-swap, noexcept

| Deliverable | Implemented in |
|---|---|
| ✅ a scope guard that rolls back unless dismissed | `ScopeGuard` |
| ✅ a noexcept swap used for commit | `Ledger::swap` |
| ✅ the unsafe version that can lose money | `Ledger::transfer_unsafe` |
| ✅ the basic guarantee: valid, but maybe changed | `Ledger::transfer_basic` |
| ✅ the strong guarantee via copy-and-swap | `Ledger::transfer_strong` |

## Key learnings
- The basic guarantee keeps every invariant intact after an exception; the strong guarantee also leaves the state exactly as before; nothrow means the operation cannot fail at all.
- A scope guard is RAII for undo: its destructor runs the rollback unless the success path calls `dismiss()`.
- Copy-and-swap gives the strong guarantee: do all the work on a copy, then commit with a swap that cannot throw.
- Marking `swap` `noexcept` documents and enforces that the commit step can never fail, and lets the standard library use it safely.

## Pitfalls I hit (and how I fixed them)
- The first transfer subtracted from one account, then threw while writing the journal – 25.00 vanished; the unsafe version is kept as a warning.
- The failure-injection hook stayed armed after a failed strong transfer, because the copy consumed it but the original kept it; it is now handed to the copy with `std::exchange`.
- A rollback lambda captured `cents` by reference to a loop variable that changed later; the guards now capture only what lives long enough.

## Run it
```bash
./procpp.sh 50                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_50_exception_safety       # the interactive demo
ctest --test-dir build -R test_day_50 --output-on-failure
```

## Next step
- Day 51 parses and produces JSON, with errors that point at the exact character.
