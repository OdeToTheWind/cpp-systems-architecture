# Day 10 – Randomisation in C++ (using <random>)

**Date:** March 23, 2026  
**Status:** Completed

## Goals
- Learn modern C++ random number generation using `<random>` (not old `rand()`)
- Understand different random engines and distributions
- Generate random integers, floating-point numbers, and booleans from user input
- Build small interactive games and simulations
- Avoid common pitfalls of the old `rand()` / `srand()` approach

## What I Learned / Key Takeaways

### Modern Approach (C++11+)
- Use `std::random_device`, `std::mt19937` (Mersenne Twister), or `std::default_random_engine`
- Separate **engine** (produces raw randomness) from **distribution** (shapes the output)
- Common distributions:
  - `std::uniform_int_distribution` → random integers in range
  - `std::uniform_real_distribution` → random floats/doubles
  - `std::bernoulli_distribution` → true/false with probability

### Why `<random>` is better than `rand()`
- Much higher quality randomness
- Thread-safe when used correctly
- Predictable and reproducible when seeded properly
- No need for `srand(time(NULL))` hacks

## Code Highlights

Main file: `src/day_10_randomisation/main.cpp`
- All parameters (range, count, probability) taken from user input
- Random number guessing game
- Dice roller, random password generator, coin flip simulator

Test file: `tests/test_day_10.cpp`
- Basic distribution range and probability tests

## Challenges / Mistakes Fixed

- Used old `rand()` initially → switched to modern `<random>`
- Forgot to seed properly → used `std::random_device`
- Inclusive/exclusive range confusion → clarified with `uniform_int_distribution`
- Non-deterministic test behavior → tests focus on distribution properties

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 11)

Loops begin:
- `while`, `do-while`, `for`
- `break` and `continue`
- First loop-based programs (sum calculator, number guessing with attempts, etc.)

Randomisation opens the door to games, simulations, and Monte Carlo methods — exciting milestone!