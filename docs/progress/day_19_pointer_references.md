# Day 19 – Pointers and References in C++

**Date:** April 01, 2026  
**Status:** Completed

## Goals
- Understand the difference between pointers (`*`) and references (`&`)
- Learn how to declare, initialize, and use pointers and references
- Master pass-by-value vs pass-by-pointer vs pass-by-reference
- Practice modifying original variables through pointers/references
- Build awareness of memory addresses and dereferencing

## What I Learned / Key Takeaways

### References (`&`)
- Alias for an existing variable
- Must be initialized at declaration
- Cannot be null or reseated
- Preferred for function parameters when you want to modify the original or avoid copying

### Pointers (`*`)
- Store memory address of a variable
- Can be null, reseated, and point to different variables
- Require dereferencing (`*`) to access the value
- More powerful but more dangerous (null pointer dereference, dangling pointers)

### Key Rules
- Pass-by-reference (`T&`) → modify original, no copy
- Pass-by-const-reference (`const T&`) → read-only, no copy
- Pass-by-pointer (`T*`) → optional modification, can be null
- Always initialize pointers (`nullptr` is best)

## Code Highlights

Main file: `src/day_19_pointer_references/main.cpp`
- All values from user input
- Demonstrates swapping with references vs pointers
- Modifying variables through pointers/references
- Pass-by-value vs pass-by-reference comparison

Test file: `tests/test_day_19.cpp`
- Tests reference and pointer behavior

## Challenges / Mistakes Fixed

- Forgetting to initialize pointers → used `nullptr`
- Confused `&` (address-of) vs reference syntax → clarified clearly
- Accidental pass-by-value when modification was needed → fixed with references
- Null pointer risks → added safety checks

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 20)

Advanced Function Topics
- `std::function`, function pointers
- Lambda expressions
- Returning functions and callbacks

Pointers and references are fundamental to understanding how C++ manages memory and data efficiently. Great work reaching this point!