# Day 08 – Conditional Statements (if / else / else if)

**Date:** March 27, 2026  
**Status:** Completed

## Goals
- Learn to use `if`, `else if`, `else` for decision making
- Understand relational (`== != < > <= >=`) and logical operators (`&& || !`)
- Practice nested conditionals and good formatting
- Build simple decision-based programs with user input
- Handle edge cases and invalid inputs gracefully

## What I Learned / Key Takeaways

- `if (condition) { ... } else if (condition) { ... } else { ... }`
- Conditions must evaluate to `bool` (or convertible to bool)
- Relational operators return `bool`
- Logical operators short-circuit (`&&` stops on false, `||` on true)
- Braces `{}` recommended even for single statements (prevents bugs)
- Prefer early returns or guard clauses for invalid input
- Nested `if` can become hard to read → keep depth low

## Code Highlights

Main file: `src/day_08_if_else_conditionals/main.cpp`
- All logic driven by user input (age, score, temperature, etc.)
- Grading system, temperature advice, login attempt checker
- Input validation with early returns

Test file: `tests/test_day_08.cpp`
- Tests for grading logic and temperature category functions

## Challenges / Mistakes Fixed

- Forgot braces around multi-statement blocks → potential bug
- Missing input validation → added checks
- Deep nesting → refactored to flatter structure where possible

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 09)

More control flow:
- `switch` statement
- `break`, `continue`, `goto` (avoid last one)
- Ternary operator `?:`
- First loop constructs (`while`, `do-while`)

Decision making is where programs start to become "smart" — great milestone!