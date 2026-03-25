# Day 12 – Functions in C++

**Date:** March 25, 2026  
**Status:** Completed

## Goals
- Understand function declaration, definition, and calling
- Learn parameter passing: pass-by-value vs pass-by-reference
- Return values and multiple return techniques
- Function overloading
- Organize code using helper functions
- Practice writing clean, reusable functions with user input

## What I Learned / Key Takeaways

- Functions improve code readability, reusability, and maintainability
- **Pass-by-value**: copy is made (safe but slower for large objects)
- **Pass-by-reference** (`&`): no copy, can modify original
- **Pass-by-const-reference** (`const T&`): best for large objects you don't modify
- Return types: `void`, fundamental types, `std::string`, etc.
- Function overloading: same name, different parameters
- Good practice: small, single-responsibility functions

## Code Highlights

Main file: `src/day_12_functions/main.cpp`
- All inputs from user
- Helper functions for calculations (power, factorial, BMI, rectangle area/perimeter)
- Function overloading example
- Pass-by-value vs pass-by-reference demonstration

Test file: `tests/test_day_12.cpp`
- Unit tests for the helper functions

## Challenges / Mistakes Fixed

- Passed large objects by value → changed to `const&`
- No input validation in functions → added checks
- Confused pass-by-value vs reference → demonstrated both clearly

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 13)

Arrays and `std::vector`
- Fixed-size arrays
- Dynamic arrays with `std::vector`
- Basic array algorithms (sum, max, min, search)

Functions are the building blocks of all larger programs — excellent work!