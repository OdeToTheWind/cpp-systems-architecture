# Day 04 – Variable Naming Rules & Conventions

**Date:** March 23, 2026  
**Status:** Completed

## Goals
- Understand C++ naming rules (what is allowed vs. what is good)
- Learn and apply professional naming conventions (mostly snake_case + camelCase patterns)
- Practice choosing clear, intention-revealing names
- Avoid common anti-patterns that hurt readability and maintainability

## What I Learned / Key Takeaways

### Legal rules (must follow – compiler enforced)
- Names must start with letter (a–z, A–Z) or underscore `_`
- After first character: letters, digits (0–9), underscore
- Case-sensitive (`age` ≠ `Age`)
- Cannot be C++ keywords (`int`, `class`, `return`, `if`, etc.)
- No special characters except underscore (`@`, `#`, `$`, etc. forbidden)

### Recommended conventions (industry style guides)
- **Variables & functions**: `snake_case` (Google, LLVM style) or `camelCase` (Microsoft, Qt style)
- **Classes / structs / enums**: `PascalCase` / `UpperCamelCase`
- **Constants / macros**: `UPPER_SNAKE_CASE`
- **Private member variables**: often `snake_case_` with trailing underscore (Google style)
- **Avoid abbreviations** unless very common (`i`, `j`, `n` for loop counters OK; `usrCnt` → bad)
- **Be descriptive but concise** — name should reveal intent
- **Avoid Hungarian notation** in modern C++ (no `strName`, `iCount`, `pPointer`)

### Common anti-patterns seen today
- Single-letter variables outside loops (`x`, `y`, `t`)
- Overly long names that repeat type info (`int userAgeInYears`)
- Inconsistent casing across project
- Using `_` prefix for user-defined names (reserved in global namespace)

## Code Highlights

Main file: `src/day_04_variable_names/main.cpp`
- Examples of good vs. bad names
- Real-world-like variable declarations in context
- Simple program showing why good names matter (readability)

Test file: `tests/test_day_04.cpp`
- Very light — mostly compile-time name checks + comment-based

## Challenges / Mistakes Fixed

- Tried using reserved names (`class`, `int`) → compile error
- Used inconsistent casing (`user_age` vs `userAge`) → decided on snake_case for this project
- Overly cryptic names in first draft → refactored to intention-revealing

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt → no changes needed
- Built & run via `./procpp.sh`

## Next Day Preview (Day 05)

Basic operators & expressions:
- Arithmetic, assignment, increment/decrement
- Relational & logical operators
- Operator precedence & associativity
- Simple expression-based programs

Naming is one of the highest-leverage skills in programming — great job focusing on it early!