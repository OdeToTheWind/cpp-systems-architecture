# Day 92 – Test Suite for a Multi-module Library Reflection

**Date:** 2026-06-13 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_92_library_system/lesson.hpp`](../../src/day_92_library_system/lesson.hpp) · **Tests:** [`tests/test_day_92.cpp`](../../tests/test_day_92.cpp) (7 tests)

## Scenario
The *lending system of a community library*. The code is split into modules with one-way dependencies – model (plain data) <- ports (interfaces plus in-memory doubles) <- service (the rules) – each compiled separately. The test suite exercises each module on its own and the service through fakes and a fixed clock: borrowing, limits, renewals, reservation queues, fines and notifications.

## Syllabus deliverables
> Separate model, repository, notifier and service modules, fakes and a fixed clock, business-rule tests

| Deliverable | Implemented in |
|---|---|
| ✅ dates as day numbers with calendar conversion | `Date::from_ymd` |
| ✅ the repository port and its in-memory fake | `InMemoryRepository` |
| ✅ a fixed clock for deterministic due dates | `FixedClock` |
| ✅ borrowing rules: limits, overdue checks, held copies | `LendingService::borrow` |
| ✅ returns with fines and reservation notices | `LendingService::return_book` |

## Key learnings
- Modules with one-way dependencies – model, then ports, then service – keep the rules independent of storage and e-mail.
- Ports are interfaces; the in-memory repository, recording notifier and fixed clock are fakes that make every rule testable.
- Storing dates as day numbers turns due dates and fines into integer arithmetic; conversion to the calendar happens only for display.
- A suite organised by module (model, ports, service) shows at a glance what is covered.

## Pitfalls I hit (and how I fixed them)
- A free copy was lent to a walk-in while someone was waiting for it; copies are now held for the reservation queue in order.
- Renewals extended from today instead of from the due date, which rewarded late renewals.
- Notifications first went out before the loan was removed, so the reader was told about a book still on loan.

## Run it
```bash
./procpp.sh 92                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_92_library_system       # the interactive demo
ctest --test-dir build -R test_day_92 --output-on-failure
```

## Next step
- Day 93 builds the core of a network service.
