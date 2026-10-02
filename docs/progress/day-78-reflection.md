# Day 78 – Unit Testing & Test Doubles Reflection

**Date:** 2026-05-30 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_78_testing/lesson.hpp`](../../src/day_78_testing/lesson.hpp) · **Tests:** [`tests/test_day_78.cpp`](../../tests/test_day_78.cpp) (7 tests)

## Scenario
The *loyalty-points service* of a coffee chain. Awarding points depends on today's date (double points on Fridays), a customer database and an email service – none of which a unit test should touch. The service receives its dependencies through interfaces (dependency injection), so tests swap in a stub clock, a fake in-memory database and a mock mailer that records what it was asked to send.

## Syllabus deliverables
> Test fixtures, table-driven tests, fakes, stubs and mocks, dependency injection

| Deliverable | Implemented in |
|---|---|
| ✅ pure rules that suit table-driven tests | `tier_for` |
| ✅ dependencies expressed as interfaces | `CustomerRepository` |
| ✅ a service that receives its dependencies (dependency injection) | `LoyaltyService` |
| ✅ a stub clock with a fixed answer | `FixedClock` |
| ✅ a fake in-memory repository | `InMemoryRepository` |
| ✅ a mock mailer that records and verifies calls | `RecordingMailer` |

## Key learnings
- Dependency injection – passing collaborators in through the constructor – is what makes code unit-testable.
- A stub returns canned answers, a fake is a simplified working implementation, and a mock records calls so a test can check how a dependency was used.
- A fixture builds a fresh, known world for every test, so tests never depend on each other.
- Table-driven tests list inputs and expected outputs as data, which makes boundary cases easy to see and to add.

## Pitfalls I hit (and how I fixed them)
- A test that read the real date only failed on Fridays; the clock is now an injected interface.
- Over-specified mocks broke whenever the implementation changed; tests verify only the interactions that matter (one email, to the right person).
- A failing email after the save left points awarded without notification – the test now documents that ordering decision explicitly.

## Run it
```bash
./procpp.sh 78                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_78_testing       # the interactive demo
ctest --test-dir build -R test_day_78 --output-on-failure
```

## Next step
- Day 79 turns code into a library others can install.
