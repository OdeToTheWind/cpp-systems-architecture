# Day 20 – Returning Functions in C++

**Date:** April 02, 2026  
**Status:** Completed

## Goals
- Learn how to return functions from other functions
- Master `std::function` wrapper
- Understand function pointers
- Use lambda expressions as return values
- Build flexible callback systems and strategy patterns

## What I Learned / Key Takeaways

- C++ can return functions using:
  - `std::function<ReturnType(Args...)>` (most flexible, recommended)
  - Function pointers (`ReturnType(*)(Args...)`)
  - Lambdas (with `auto` or `std::function`)
- `std::function` can wrap lambdas, function pointers, and functors
- Returning functions enables **strategy pattern**, callbacks, and higher-order programming
- Capturing variables in lambdas (`[&]`, `[=]`, `[x]`) is powerful but requires care with lifetimes

## Code Highlights

Main file: `src/day_20_returning_functions/main.cpp`
- All inputs from user
- Returns different math operations based on user choice
- Dynamic callback system for calculations
- Strategy pattern example (different pricing rules)

Test file: `tests/test_day_20.cpp`
- Tests returned function objects and lambdas

## Challenges / Mistakes Fixed

- Returned raw function pointers → switched to safer `std::function`
- Lifetime issues with lambdas → used proper capture modes
- Complex syntax → simplified with clear examples

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 21)

Return vs Print + Side Effects
- Pure functions vs functions with side effects
- When to return values vs when to print
- Designing better APIs

Being able to return and pass around functions is a powerful technique that unlocks many advanced patterns in C++.