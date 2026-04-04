# Day 21 – Return vs Print (Pure Functions vs Side Effects)

**Date:** April 03, 2026  
**Status:** Completed

## Goals
- Understand the difference between returning a value and printing it
- Learn pure functions vs functions with side effects
- Practice designing better APIs by preferring return values
- Know when printing is appropriate (logging, user output)

## What I Learned / Key Takeaways

- **Return** = Give data back to the caller (functional style)
- **Print** = Side effect (changes the outside world)
- Pure functions (always return same output for same input, no side effects) are easier to test and reason about
- Mixing return and print in the same function often leads to poor design
- Good practice: Functions should either compute and return, or perform I/O — rarely both

## Code Highlights

Main file: `src/day_21_return_vs_print/main.cpp`
- Interactive calculator showing return-based vs print-based design
- Comparison of good vs bad function design

Test file: `tests/test_day_21.cpp`
- Tests pure functions

**Key Lesson**: Prefer returning values. Let the caller decide what to do with the result (print, save, use in calculation, etc.).