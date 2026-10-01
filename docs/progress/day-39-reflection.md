# Day 39 – Inheritance Reflection

**Date:** 2026-04-21 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_39_inheritance/lesson.hpp`](../../src/day_39_inheritance/lesson.hpp) · **Tests:** [`tests/test_day_39.cpp`](../../tests/test_day_39.cpp) (7 tests)

## Scenario
An *electric-vehicle charging network*. A base `Charger` defines how a charging session is billed; AC posts, DC fast chargers and solar-canopy chargers specialise it, and capability interfaces (remote reporting, maintenance) are mixed in with multiple inheritance.

## Syllabus deliverables
> Base and derived classes, virtual and override, final, virtual destructors, multiple inheritance of interfaces

| Deliverable | Implemented in |
|---|---|
| ✅ a base class with a non-virtual template method | `Charger::session_cost_cents` |
| ✅ virtual hooks overridden in derived classes | `AcCharger` |
| ✅ a final class that cannot be derived from further | `DcFastCharger` |
| ✅ multiple inheritance of pure interfaces | `SolarCanopyCharger` |
| ✅ virtual destructors observed through a base pointer | `Charger::~Charger` |
| ✅ asking for a capability at run time | `maintenance_due` |

## Key learnings
- A non-virtual 'template method' in the base class fixes the procedure, while protected virtual hooks let each derived class supply the details.
- `override` makes the compiler check that a function really overrides a base virtual function, and `final` forbids further overriding or derivation.
- A base class used polymorphically needs a virtual destructor, or deleting through a base pointer skips the derived destructor.
- Multiple inheritance is safe and common when the extra bases are pure interfaces; `dynamic_cast` can ask an object whether it implements one.

## Pitfalls I hit (and how I fixed them)
- A misspelled `price_per_kwh_cent()` silently created a new function instead of overriding; adding `override` turned that into a compile error.
- Destroying a fast charger through `unique_ptr<Charger>` skipped the derived destructor until `~Charger` became virtual.
- The session cost exceeded what a charger can physically deliver; the base class now caps the energy by power and time.

## Run it
```bash
./procpp.sh 39                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_39_inheritance       # the interactive demo
ctest --test-dir build -R test_day_39 --output-on-failure
```

## Next step
- Day 40 looks at how containers are walked – iterators.
