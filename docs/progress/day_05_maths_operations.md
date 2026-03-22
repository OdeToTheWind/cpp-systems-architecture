# Day 05 – Mathematical Operations in C++

**Date:** March 18, 2026  
**Status:** Completed

## Goals
- Understand and use all basic arithmetic operators interactively
- Practice compound assignment operators (`+=`, `-=`, `*=`, `/=`, `%=`)
- Compare prefix vs postfix increment/decrement with user input
- Apply `<cmath>` functions on values provided by the user
- Build interactive mini-programs that demonstrate math concepts

## What I Learned / Key Takeaways

- Integer vs floating-point division behavior
- Safe handling of division by zero
- Compound assignment makes code more concise
- Prefix `++x` vs postfix `x++` changes when the value is used
- `<cmath>` functions (`sqrt`, `pow`, `hypot`, `round`, `abs`, etc.) are type-aware
- Operator precedence: `* / %` before `+ -`, parentheses always improve clarity
- Taking all values from input makes demos more dynamic and testable

## Code Highlights

Main file: `src/day_05_maths_operations/main.cpp`
- All examples now interactive (user inputs a, b, price, counter, x, radius)
- Division-by-zero protection
- Circle calculator with diameter added
- Formatted output with `fixed` and `setprecision`

Test file: `tests/test_day_05.cpp`
- Tests focus on pure math logic (independent of I/O)

## Challenges / Mistakes Fixed

- Division by zero → added check
- Floating-point precision → used `std::fixed` and `setprecision(2)`
- Hard-coded values replaced with user input for better interactivity
- Input validation and error recovery for invalid user inputs (e.g., non-numeric or comma-separated values)

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 06)

Data types in C++:
- Fundamental (built-in) data types (`int`, `float`, `double`, `bool`, `char`, etc.)
- Type modifiers (`signed`, `unsigned`, `short`, `long`, `long long`)
- Type sizes, ranges, and platform dependencies using `<cstdint>` and `<limits>`
- Type aliases (`using` / `typedef`) for clarity and code safety

Great step forward — interactive math makes concepts stick better!