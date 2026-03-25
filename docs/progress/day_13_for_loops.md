# Day 13 – For Loops in C++

**Date:** March 26, 2026  
**Status:** Completed

## Goals
- Master the `for` loop syntax and its three parts (init, condition, increment)
- Understand range-based for loops (`for (auto x : container)`)
- Use loops for repetitive tasks with user input
- Combine loops with conditionals and functions
- Learn common loop patterns (sum, factorial, counting, printing patterns)

## What I Learned / Key Takeaways

- Traditional `for` loop: `for (init; condition; increment)`
- Range-based for loop (C++11+): cleaner for iterating containers/arrays
- `break` → exit loop early
- `continue` → skip current iteration
- Avoid modifying loop counter inside the loop body
- Nested loops are powerful but can become complex quickly
- Always validate user input before entering loops

## Code Highlights

Main file: `src/day_13_for_loops/main.cpp`
- All values (count, range, number) taken from user input
- Sum of first N numbers, factorial, multiplication table, star pattern printer
- Range-based for loop with `std::vector`

Test file: `tests/test_day_13.cpp`
- Tests for sum, factorial, and loop-based calculations

## Challenges / Mistakes Fixed

- Infinite loop due to wrong condition → fixed with proper bounds
- Off-by-one errors → carefully handled start/end values
- Used old C-style loops → added range-based examples
- No input validation → added checks before loops

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 14)

While and do-while loops + more loop patterns
- Difference between `for`, `while`, and `do-while`
- Menu-driven programs with loops
- First simple games and interactive tools

Loops are where real computation power begins — you're making excellent progress!