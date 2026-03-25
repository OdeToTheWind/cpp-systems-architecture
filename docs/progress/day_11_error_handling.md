# Day 11 – Error Handling (try / catch / throw)

**Date:** March 24, 2026  
**Status:** Completed

## Goals
- Understand exception handling mechanism in C++ (`try`, `catch`, `throw`)
- Learn how to throw and catch different types of exceptions
- Handle runtime errors gracefully (division by zero, invalid input, out-of-range, etc.)
- Use `std::exception` hierarchy and custom exceptions
- Practice robust input validation using exceptions

## What I Learned / Key Takeaways

- Exceptions are used for **exceptional** (rare/unexpected) situations
- `try { ... } catch (const std::exception& e) { ... }`
- `throw` keyword to signal error
- Multiple `catch` blocks — more specific exceptions first
- `std::runtime_error`, `std::invalid_argument`, `std::out_of_range`
- Catching by `const reference` is best practice
- Exceptions unwind the stack — resources should be managed with RAII (later topic)

### Best Practices
- Don't use exceptions for normal control flow
- Catch specific exceptions before general ones
- Always catch `std::exception` as base class last
- Provide meaningful error messages

## Code Highlights

Main file: `src/day_11_error_handling/main.cpp`
- All inputs from user
- Safe division with exception
- Age validation, array access with bounds checking
- Custom exception for invalid password

Test file: `tests/test_day_11.cpp`
- Tests exception throwing and catching logic

## Challenges / Mistakes Fixed

- Used `throw "string"` → replaced with proper exception objects
- Caught exceptions by value → changed to `const reference`
- No re-throwing demonstration → added `throw;` example
- Forgot to include `<stdexcept>` → added proper headers

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 12)

Loops introduction:
- `while`, `do-while`, `for` loops
- `break` and `continue`
- First loop-based algorithms (sum of numbers, factorial, prime check, etc.)

Proper error handling makes your programs robust and professional — important milestone!