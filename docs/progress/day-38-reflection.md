# Day 38 – Game Development with OOP Reflection

**Date:** 2026-04-20 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_38_game_development/lesson.hpp`](../../src/day_38_game_development/lesson.hpp) · **Tests:** [`tests/test_day_38.cpp`](../../tests/test_day_38.cpp) (8 tests)

## Scenario
*Dungeon Duel*, a turn-based battle. A hero fights a sequence of monsters; both sides attack, the hero can drink a limited number of potions, and the game can be won or lost. Randomness is injected through the engine, so every battle can be replayed in tests.

## Syllabus deliverables
> Game entities as classes, turn-based game loop, injected randomness, win and lose conditions

| Deliverable | Implemented in |
|---|---|
| ✅ a base class for every combatant | `Combatant` |
| ✅ the player's character with extra abilities | `Hero` |
| ✅ enemies that specialise the base class | `Monster` |
| ✅ randomness injected through a seeded engine | `Battle::Battle` |
| ✅ one turn of the game loop | `Battle::play_turn` |
| ✅ win and lose conditions | `Battle::outcome` |

## Key learnings
- Game entities share a base class (`Combatant`) for what they have in common and override only what differs, such as how an ogre attacks.
- A turn-based game loop is a state machine: act, react, then check the win and lose conditions.
- Injecting the random engine with a seed makes every battle replayable, which is what makes game logic testable.
- `std::unique_ptr<Monster>` in a vector lets one dungeon hold different monster types polymorphically.

## Pitfalls I hit (and how I fixed them)
- Health could go negative and display as -7 hp; `take_damage` now clamps at zero and ignores negative damage.
- A monster that had just died still hit back in the same turn; the loop now checks `alive()` before the counter-attack.
- Using a global random engine made tests depend on test order; each battle now owns its seeded engine.

## Run it
```bash
./procpp.sh 38                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_38_game_development       # the interactive demo
ctest --test-dir build -R test_day_38 --output-on-failure
```

## Next step
- Day 39 looks more closely at inheritance hierarchies and their rules.
