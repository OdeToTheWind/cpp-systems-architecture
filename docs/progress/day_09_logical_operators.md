# Day 09 – Logical Operators (&& || !)

**Date:** March 22, 2026  
**Status:** Completed

## Goals
- Master logical AND (`&&`), OR (`||`), and NOT (`!`)
- Understand short-circuit evaluation behavior
- Combine logical operators with relational operators
- Write complex conditions using user input
- Practice De Morgan's laws and condition simplification

## What I Learned / Key Takeaways

- `&&` (AND): true only if **both** operands are true — short-circuits on first false
- `||` (OR): true if **at least one** operand is true — short-circuits on first true
- `!` (NOT): reverses the truth value
- Short-circuiting → right operand may not be evaluated (important for side effects)
- Precedence: `!` > `&&` > `||` — use parentheses for clarity
- De Morgan's laws:
  - `!(A && B)` ≡ `!A || !B`
  - `!(A || B)` ≡ `!A && !B`
- Logical operators return `bool` — very useful in `if` conditions

## Code Highlights

Main file: `src/day_09_logical_operators/main.cpp`
- All conditions built from user input
- Login eligibility, discount eligibility, password strength, range check
- Demonstrates short-circuiting with safe division example

Test file: `tests/test_day_09.cpp`
- Tests combined logical expressions

## Challenges / Mistakes Fixed

- Confused precedence (`!a && b` vs `!(a && b)`) → added parentheses
- Forgot short-circuiting can prevent crashes → used in safe division
- Overly complex conditions → refactored into named bool variables

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 10)

Randomization (Modern C++):
- Introduction to the `<random>` header
- Seeding with `std::random_device`
- Using a robust random number engine (`std::mt19937` Mersenne Twister)
- Applying distributions (e.g., `std::uniform_int_distribution`, `std::uniform_real_distribution`)
- Why modern `<random>` is preferred over the legacy `rand()` and `srand()` functions

Logical operators let you express complex decisions cleanly — very powerful tool!