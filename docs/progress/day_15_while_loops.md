# Day 15 – While and Do-While Loops

**Date:** March 28, 2026  
**Status:** Completed

## Goals
- Understand the difference between `while` and `do-while` loops
- Learn when to use each type of loop
- Practice writing loops that depend on user input
- Combine loops with conditionals and functions
- Handle user-controlled repetition (menu systems, games, validation)

## What I Learned / Key Takeaways

### While Loop
- Checks condition **before** executing the body
- Body may never execute if condition is initially false
- Most commonly used loop when number of iterations is unknown

### Do-While Loop
- Executes body **at least once**, then checks condition
- Perfect for menus and "do you want to continue?" patterns

### Key Differences
- `while (condition) { ... }` → pre-test
- `do { ... } while (condition);` → post-test
- Use `break` to exit early
- Use `continue` to skip to next iteration

## Code Highlights

Main file: `src/day_15_while_loops/main.cpp`
- All behavior driven by user input
- Number guessing game with attempt limit
- Interactive menu system using do-while
- Input validation loop
- Sum calculator until user chooses to stop

Test file: `tests/test_day_15.cpp`
- Tests loop logic and edge cases

## Challenges / Mistakes Fixed

- Infinite loops due to missing input update → fixed with proper condition
- Used `while` when `do-while` was better for menu → corrected
- Forgot to clear input buffer → added `cin.ignore()`
- Deep nesting → refactored using functions where appropriate

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 16)

Arrays and `std::vector`
- Fixed-size arrays (`[]`)
- Dynamic arrays with `std::vector`
- Basic array operations and algorithms

Loops give your programs the power to repeat actions — you're now entering the realm of real algorithms!