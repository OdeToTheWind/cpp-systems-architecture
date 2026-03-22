# Day 02 – String Manipulation (std::string)

**Date:** March 15, 2026  
**Status:** Completed

## Goals
- Master `std::string` member functions
- Case-insensitive palindrome checker
- String mutation and immutability patterns

## What I Learned

- `.length()`, `.empty()`, `.front()`, `.back()`, `.find()`, `.substr()`
- `std::getline` vs `cin >>`
- `std::reverse`, `std::tolower`, `std::isalpha`
- Safe casting for `tolower`/`isalpha`
- Extracting testable logic to header (`string_utils.hpp`)

## Code Highlights

- `src/day_02_strings/main.cpp` → interactive demo
- `src/day_02_strings/string_utils.hpp` → `isPalindrome`
- `tests/test_day_02.cpp` → assert-based tests

## Challenges Fixed
- Missing `<algorithm>` include
- Unsafe `tolower` usage → fixed with `static_cast<unsigned char>`
- `getline` after `>>` issue (not in day 2 but learned nearby)
- Test file compilation error → solved by moving function to header

## Notes
CI (GitHub Actions) turned green after MSYS2 setup + caching!