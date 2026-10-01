# Day 01 – Variables, Types & Basic I/O Reflection

**Date:** 2026-03-14 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_01_variables/lesson.hpp`](../../src/day_01_variables/lesson.hpp) · **Tests:** [`tests/test_day_01.cpp`](../../tests/test_day_01.cpp) (10 tests)

## Scenario
A *coding-club sign-up desk* that records each new member's profile card (name, age, height, membership tier, newsletter choice) and offers a mini calculator for splitting the club's membership fees.

## Syllabus deliverables
> Fundamental types, brace initialisation, auto, const, reading values with std::cin and std::getline, a mini calculator

| Deliverable | Implemented in |
|---|---|
| ✅ fundamental types in one record | `MemberCard` |
| ✅ brace initialisation, auto and const | `type_tour` |
| ✅ parsing a whole line into a number | `parse_number` |
| ✅ reading values with std::getline | `read_member` |
| ✅ a mini calculator | `calculate` |

## Key learnings
- Brace initialisation (`int x{3.7};`) refuses narrowing conversions at compile time, while `int x = 3.7;` silently truncates.
- `auto` deduces the type from the initialiser, so `auto fee = 12.5;` is a `double` and `auto name = std::string{"x"};` is a `std::string`, not a `const char*`.
- `std::cin >> x` stops at whitespace, so names with spaces need `std::getline`; reading every answer as a whole line and then parsing it avoids the classic leftover-newline bug.
- A failed extraction puts the stream into a fail state; a robust reader checks the result and asks again instead of using a half-read value.

## Pitfalls I hit (and how I fixed them)
- Reading the age with `std::cin >> age` and then the name with `std::getline` produced an empty name – reading every answer with `std::getline` and parsing fixed it.
- `"12abc"` was accepted as 12 by `>>`; `parse_number` now requires the whole line to be the number.
- The calculator used to print `inf` for division by zero; `calculate` now throws `std::domain_error` and the demo reports it.

## Run it
```bash
./procpp.sh 1                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_01_variables       # the interactive demo
ctest --test-dir build -R test_day_01 --output-on-failure
```

## Next step
- Day 02 turns the raw strings typed at the sign-up desk into clean, well-formatted text.
