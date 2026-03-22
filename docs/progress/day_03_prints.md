# Day 03 – Input and Print Functions (cin / cout)

**Date:** March 22, 2026  
**Status:** Completed

## Goals
- Master console input/output patterns with `cin` and `cout`
- Handle different data types (numbers, strings with spaces)
- Clean input buffer after `>>` to safely use `getline`
- Add gender input and show basic conditional formatting
- Practice formatted output with `<iomanip>`

## What I Learned / Key Takeaways

- `std::cin >>` stops at whitespace; good for numbers/words
- `std::getline(std::cin, str)` reads full lines (including spaces)
- Critical: use `std::cin.ignore()` after `>>` to remove leftover newline
- `std::setw`, `std::left`, `std::fixed`, `std::setprecision` for nice tables
- Input validation: check `cin.fail()` and positive values
- Gender input → simple char → string mapping + `tolower` for flexibility

## Code Highlights

Main file: `src/day_03_prints/main.cpp`
- Age (int), full name (string), gender (char → string), height & weight (double)
- BMI calculation + formatted profile output
- Basic input error handling and buffer cleanup

Test file: `tests/test_day_03.cpp`
- Simulation of BMI logic (extracted calculation)

## Challenges / Mistakes Fixed

- `getline` reading empty line after `cin >>` → fixed with `cin.ignore(numeric_limits<streamsize>::max(), '\n')`
- No validation for height/weight → added checks
- Gender input case insensitivity → used `tolower`
- Missing `<cctype>` for `tolower`

## CMake & Build Notes

- Automatically detected by CMakeLists.txt
- Run: `./procpp.sh` → `./build/day_03_prints.exe`

## Next Day Preview (Day 04)

Control flow basics:
- `if`, `else if`, `else`
- `switch` statement
- Comparison & logical operators
- Simple decision programs (grading system, simple menu, etc.)

Consistency beats intensity — keep going strong!