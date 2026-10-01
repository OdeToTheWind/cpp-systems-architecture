# Day 32 – Constructors and Initialiser Lists Reflection

**Date:** 2026-04-14 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_32_constructors/lesson.hpp`](../../src/day_32_constructors/lesson.hpp) · **Tests:** [`tests/test_day_32.cpp`](../../tests/test_day_32.cpp) (7 tests)

## Scenario
A *car-rental reservation desk*. Every way of creating a reservation – a blank walk-in form, a full booking, a booking for "N days from a start day", or a copy of last year's booking – goes through a constructor that guarantees a valid object from the very first moment it exists.

## Syllabus deliverables
> Default, parameterised, delegating and copy constructors, member initialiser lists, validation at construction

| Deliverable | Implemented in |
|---|---|
| ✅ a parameterised constructor that validates everything | `Reservation::Reservation` |
| ✅ members initialised in the initialiser list, in declaration order | `Reservation::confirmation_` |
| ✅ a custom copy constructor with different copy semantics | `Reservation::copy_for_rebooking` |
| ✅ a default constructor for a blank walk-in form | `WalkInForm` |
| ✅ counting constructions to see which constructor ran | `ConstructionLog` |

## Key learnings
- The member initialiser list initialises members directly; const members and references can only be initialised there.
- Members are always initialised in declaration order, not in the order of the initialiser list – `-Wreorder` warns when the two differ.
- A delegating constructor forwards to another constructor of the same class, so the validation lives in one place.
- A user-written copy constructor can give copies different meaning – here a copy is a new booking with a new confirmation number.

## Pitfalls I hit (and how I fixed them)
- Assigning members in the constructor body default-constructed them first and failed outright for the const confirmation number.
- A copied reservation kept the original's confirmation number, so two bookings shared one; the copy constructor now issues a new one.
- `Reservation(name, car, 10, 3)` picked the wrong overload until the 'N days' constructor took an `unsigned` day count.

## Run it
```bash
./procpp.sh 32                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_32_constructors       # the interactive demo
ctest --test-dir build -R test_day_32 --output-on-failure
```

## Next step
- Day 33 organises the growing code base into namespaces.
