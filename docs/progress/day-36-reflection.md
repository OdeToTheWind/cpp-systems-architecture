# Day 36 – Instances and State Reflection

**Date:** 2026-04-18 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_36_instances_state/lesson.hpp`](../../src/day_36_instances_state/lesson.hpp) · **Tests:** [`tests/test_day_36.cpp`](../../tests/test_day_36.cpp) (7 tests)

## Scenario
A *parcel-locker network*. Every parcel object tracks its own state as it moves through a lifecycle – registered, in transit, waiting in a locker, collected – or ends up returned to the sender. Illegal jumps (collecting a parcel that never arrived) are refused, and each parcel keeps its own history.

## Syllabus deliverables
> Per-object state, state machines with enum class, transition validation, lifecycle history

| Deliverable | Implemented in |
|---|---|
| ✅ the states of the lifecycle as an enum class | `State` |
| ✅ the transition table that defines legal moves | `allowed` |
| ✅ per-object state with validated transitions | `Parcel::advance` |
| ✅ each object keeps its own history | `Parcel::history` |
| ✅ state-dependent behaviour | `Parcel::collect` |
| ✅ many independent instances managed together | `LockerNetwork` |

## Key learnings
- An `enum class` gives each state a name and stops states from silently converting to integers.
- A transition table makes the whole state machine visible in one place and turns 'is this move legal?' into one lookup.
- Every object carries its own state and history, so a thousand parcels are a thousand independent state machines.
- Behaviour that depends on state (`collect`, `expire_if_uncollected`) checks the state first and goes through the same validated transition.

## Pitfalls I hit (and how I fixed them)
- A parcel could be 'collected' before it reached the locker because the setter accepted any state; `advance` now consults the table.
- An illegal transition used to change the state before throwing; validation now happens before anything is modified.
- History entries with a day earlier than the previous one made the expiry calculation negative; time is now checked to move forward.

## Run it
```bash
./procpp.sh 36                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_36_instances_state       # the interactive demo
ctest --test-dir build -R test_day_36 --output-on-failure
```

## Next step
- Day 37 draws pictures – with a canvas object whose state is pixels.
