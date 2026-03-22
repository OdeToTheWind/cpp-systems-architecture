# Day 06 – Data Types in C++

**Date:** March 25, 2026  
**Status:** Completed

## Goals
- Understand all fundamental (built-in) data types in C++
- Learn type modifiers (`signed`, `unsigned`, `short`, `long`, `long long`)
- Explore type sizes, ranges, and platform dependencies using `<cstdint>` and `<limits>`
- Use type aliases (`using` / `typedef`) for clarity
- Observe signed vs unsigned behavior and overflow/underflow
- Build awareness of type safety and when to choose which type

## What I Learned / Key Takeaways

### Fundamental Types
- **Integer types**:
  - `char` (usually 8 bits), `signed char`, `unsigned char`
  - `short`, `int`, `long`, `long long`
  - `unsigned short`, `unsigned int`, `unsigned long`, `unsigned long long`
- **Floating-point types**:
  - `float` (~6–7 decimal digits), `double` (~15–16 digits), `long double`
- **Boolean**: `bool` (true/false)
- **Character types**: `char`, `wchar_t`, `char8_t` (C++20), `char16_t`, `char32_t`

### Fixed-width types (from `<cstdint>`) — highly recommended in 2025+
- `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- `intptr_t`, `uintptr_t`, `ptrdiff_t`, `size_t`

### Key Observations
- `sizeof(type)` gives size in bytes
- `std::numeric_limits<T>::min()` / `max()` → safe range checking
- Signed integer overflow = undefined behavior (UB) — avoid!
- Unsigned overflow wraps around (modulo 2^n)
- `float` vs `double`: prefer `double` unless memory is critical
- Modern C++: prefer fixed-width types when exact size/range matters

## Code Highlights

Main file: `src/day_06_data_types/main.cpp`
- Prints size and range of common types
- Demonstrates signed/unsigned overflow behavior
- Uses fixed-width types and `<limits>`
- Simple type alias example

Test file: `tests/test_day_06.cpp`
- Checks type sizes and basic range behavior

## Challenges / Mistakes Fixed

- Assuming `int` is always 32-bit → wrong on some platforms
- Signed overflow → UB warning added
- Mixing signed/unsigned in comparisons → implicit conversion warning

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 07)

Relational & logical operators + first control flow
- Comparison: `== != < > <= >=`
- Logical: `&& || !`
- `if`, `else if`, `else`
- First real conditional programs (grading, simple decisions)

Understanding data types deeply prevents many bugs — excellent choice for Day 6.