# Day 10 – Randomisation Reflection

**Date:** 2026-03-23 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_10_randomisation/lesson.hpp`](../../src/day_10_randomisation/lesson.hpp) · **Tests:** [`tests/test_day_10.cpp`](../../tests/test_day_10.cpp) (8 tests)

## Scenario
A *tabletop role-playing game master's toolkit*: dice notation such as `3d6+2`, loot that drops with a given chance, randomly generated non-player characters and a shuffled initiative order – all reproducible from a seed so a session can be replayed.

## Syllabus deliverables
> std::mt19937 engines, seeding, uniform, normal and bernoulli distributions, shuffling, reproducible randomness

| Deliverable | Implemented in |
|---|---|
| ✅ seeding a std::mt19937 engine (fixed or random\_device) | `make_engine` |
| ✅ parsing and validating dice notation | `parse_dice` |
| ✅ uniform\_int\_distribution for fair dice | `roll` |
| ✅ bernoulli\_distribution for drop chances | `count_loot_drops` |
| ✅ normal\_distribution for natural variation | `npc_heights` |
| ✅ std::shuffle for a fair turn order | `initiative_order` |

## Key learnings
- An *engine* (`std::mt19937`) produces raw random bits and a *distribution* shapes them; one engine can feed many distributions.
- `std::uniform_int_distribution<int>(1, 6)` includes both bounds, unlike the half-open ranges used elsewhere in the library.
- Seeding with a fixed number makes a whole session reproducible – essential for tests and for replaying a reported bug – while `std::random_device` supplies a fresh seed for real play.
- `std::shuffle` gives every permutation the same probability; hand-written swap loops usually do not.

## Pitfalls I hit (and how I fixed them)
- A minimum greater than the maximum crashed the old demo, because it violates `uniform_int_distribution`'s precondition – `parse_dice` now validates the numbers before any distribution is built.
- A loot chance of 1.5 was accepted and behaved oddly; `count_loot_drops` rejects probabilities outside [0, 1], including NaN.
- Tests that compared exact random numbers failed on another standard library, because distributions may be implemented differently – the tests now check ranges and statistics, and only compare runs made on the same platform.

## Run it
```bash
./procpp.sh 10                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_10_randomisation       # the interactive demo
ctest --test-dir build -R test_day_10 --output-on-failure
```

## Next step
- Day 11 makes the programs survive bad data with exceptions instead of crashing.
