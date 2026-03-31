# Day 18 – Positional vs Keyword Arguments in C++

**Date:** March 31, 2026  
**Status:** Completed

## Goals
- Understand positional arguments (the default in C++)
- Learn how to simulate "keyword arguments" since C++ does not support them natively
- Master default function arguments
- Explore function overloading as an alternative to keyword args
- Use the Named Parameter Idiom for flexible function calls

## What I Learned / Key Takeaways

### Positional Arguments
- Arguments are matched by position/order
- Easy but can become confusing with many parameters

### Default Arguments
- Allow optional parameters with default values
- Must be placed at the end of the parameter list

### Simulating Keyword Arguments
- **Function Overloading**: Multiple functions with same name but different parameters
- **Named Parameter Idiom**: Pass a struct/object with named fields
- **Builder Pattern** (advanced, shown briefly)

### Best Practices
- Limit number of parameters (ideally ≤ 4-5)
- Use default arguments wisely
- Prefer structs for complex configuration
- Keep function signatures clear and readable

## Code Highlights

Main file: `src/day_18_positional_keyword_arguments/main.cpp`
- All inputs from user
- Demonstrates positional arguments, default arguments, function overloading
- Shows Named Parameter Idiom using a configuration struct

Test file: `tests/test_day_18.cpp`
- Tests default arguments and overloading behavior

## Challenges / Mistakes Fixed

- Tried to use Python-style keyword args → learned C++ limitations
- Overused default arguments → showed when overloading is better
- Confusing parameter order → used struct for clarity

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 19)

Pointers and References
- Raw pointers (`*`)
- References (`&`)
- Difference between pass-by-value, pass-by-pointer, pass-by-reference
- First steps toward memory management

Understanding argument passing is crucial for writing efficient and safe C++ code.