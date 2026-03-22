# Day 07 – Converting Types (Casting) in C++

**Date:** March 20, 2026  
**Status:** Completed

## Goals
- Understand implicit (automatic) type conversion rules
- Learn explicit type casting: C-style vs modern C++ casts
- Explore `static_cast`, `dynamic_cast`, `const_cast`, `reinterpret_cast`
- Handle common conversion pitfalls (loss of precision, narrowing, undefined behavior)
- Practice safe conversion patterns with user-provided values

## What I Learned / Key Takeaways

### Implicit Conversion (Promotion / Conversion)
- Smaller → larger type (int → double, char → int)
- Integer promotion: `char`, `short` → `int` in arithmetic
- Usual arithmetic conversions: operands promoted to common type
- Floating-point to integer truncates toward zero

### Explicit Casting
- **C-style cast**: `(type)value` — dangerous, mixes behaviors
- **Modern C++ casts** (preferred):
  - `static_cast<T>(value)` — compile-time, safe for related types
  - `dynamic_cast<T>(ptr)` — runtime, for polymorphic types (later)
  - `const_cast<T>(value)` — add/remove const/volatile
  - `reinterpret_cast<T>(value)` — low-level, dangerous (pointer reinterpretation)

### Important Rules & Pitfalls
- Narrowing conversion (double → int) → warning with `-Wnarrowing`
- Signed ↔ unsigned conversion → can produce very large values
- Floating-point precision loss when converting to integer
- Avoid `reinterpret_cast` unless doing low-level work

## Code Highlights

Main file: `src/day_07_convert_types_casting/main.cpp`
- All values taken from user input
- Demonstrates implicit conversion, safe static_cast, narrowing warning case
- Shows signed/unsigned pitfalls and safe alternatives

Test file: `tests/test_day_07.cpp`
- Tests conversion logic and boundary behavior

## Challenges / Mistakes Fixed

- C-style cast used initially → replaced with `static_cast`
- Signed/unsigned mismatch warning → handled with explicit cast
- Floating-point to int truncation → demonstrated and warned about

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 08)

Control flow begins — `if`, `else`, `switch`
- First real conditional logic
- Comparison & logical operators in action
- Simple decision-making programs

Mastering casting prevents subtle bugs — great topic for early days.