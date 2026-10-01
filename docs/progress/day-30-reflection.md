# Day 30 – Getters and Setters Reflection

**Date:** 2026-04-12 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_30_getters_setters/lesson.hpp`](../../src/day_30_getters_setters/lesson.hpp) · **Tests:** [`tests/test_day_30.cpp`](../../tests/test_day_30.cpp) (7 tests)

## Scenario
An *aquarium controller*. The keeper can read and set the water temperature in Celsius or Fahrenheit and adjust the pH and lighting, but the controller refuses any value that would harm the fish – and stores temperatures in one exact internal unit.

## Syllabus deliverables
> Accessors and mutators, validation in setters, unit conversion behind an interface

| Deliverable | Implemented in |
|---|---|
| ✅ const getters that never change the object | `AquariumController::temperature_c` |
| ✅ a getter that converts units on the way out | `AquariumController::temperature_f` |
| ✅ a setter that validates before it changes anything | `AquariumController::set_temperature_c` |
| ✅ a setter in a second unit reusing the first | `AquariumController::set_temperature_f` |
| ✅ a validated setter with a different range | `AquariumController::set_ph` |
| ✅ an audit trail of every accepted change | `AquariumController::history` |

## Key learnings
- Getters are `const` member functions, so they can be called on const objects and promise not to change anything.
- A setter is the single place where validation happens; if it throws before changing a member, a refused value leaves the object untouched.
- Storing one canonical unit inside the class (tenths of a degree) and converting in the getters keeps the interface flexible and the data exact.
- A second setter in another unit should convert and delegate, so the rules exist in exactly one function.

## Pitfalls I hit (and how I fixed them)
- Storing the temperature as a `double` made 25.3 come back as 25.299999; whole tenths stored in an `int` round-trip exactly.
- A NaN temperature passed the range check because every comparison with NaN is false; `std::isfinite` is checked first.
- The Fahrenheit setter had its own copy of the limits, which drifted out of sync; it now converts and calls the Celsius setter.

## Run it
```bash
./procpp.sh 30                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_30_getters_setters       # the interactive demo
ctest --test-dir build -R test_day_30 --output-on-failure
```

## Next step
- Day 31 looks at the different kinds of member functions: const, static and chainable.
