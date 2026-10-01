# Day 15 – While and Do-While Loops Reflection

**Date:** 2026-03-28 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_15_while_loops/lesson.hpp`](../../src/day_15_while_loops/lesson.hpp) · **Tests:** [`tests/test_day_15.cpp`](../../tests/test_day_15.cpp) (8 tests)

## Scenario
A *vending-machine controller*. It accepts coins until the price is covered (or the customer types `cancel`), pays change with as few coins as possible, locks the service panel after three wrong PINs, and shows its menu at least once per session.

## Syllabus deliverables
> Pre-test and post-test loops, sentinel loops, menu loops, termination on end of input

| Deliverable | Implemented in |
|---|---|
| ✅ pre-test while loop that may run zero times | `make_change` |
| ✅ while loop with an unknown number of steps | `collatz_steps` |
| ✅ sentinel loop that also stops at end of input | `collect_coins` |
| ✅ bounded retries with a while loop | `unlock_panel` |
| ✅ do-while menu shown at least once | `run` |

## Key learnings
- A `while` loop tests before every iteration and may run zero times; a `do … while` loop runs its body at least once – a natural fit for a menu.
- A sentinel value such as `cancel` ends an input loop cleanly, but end of input must end it too, or the loop spins forever.
- Bounded retries need an explicit counter in the condition (`attempts < max_attempts`), not just a hope that the user gets it right.
- Some loops have no predictable length – the Collatz sequence is the classic example – which is exactly when `while` beats `for`.

## Pitfalls I hit (and how I fixed them)
- The old menu asked for two numbers before checking whether the menu choice was valid; the choice is now validated first.
- Closing the input in the middle of the coin loop made the program loop forever; `collect_coins` now treats end of input like `cancel` and refunds the coins.
- Greedy change for an amount the coins cannot make (3 cents) never terminated in a draft – the loop now also stops when it runs out of coin sizes.

## Run it
```bash
./procpp.sh 15                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_15_while_loops       # the interactive demo
ctest --test-dir build -R test_day_15 --output-on-failure
```

## Next step
- Day 16 draws these loops and decisions as flowcharts before writing them.
