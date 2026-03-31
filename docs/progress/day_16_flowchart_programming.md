# Day 16 – Flowchart Programming

**Date:** March 29, 2026  
**Status:** Completed

## Goals
- Understand how to convert flowcharts into working C++ code
- Practice translating decision diamonds, process rectangles, and loops into `if/else` and loop constructs
- Develop the habit of thinking in flowcharts before writing code
- Build several common flowchart-based programs using user input
- Improve problem-solving by visualizing logic flow

## What I Learned / Key Takeaways

- Flowcharts help visualize program logic before coding
- Common symbols:
  - Oval → Start/End
  - Rectangle → Process
  - Diamond → Decision (if/else)
  - Arrow → Flow direction
  - Parallelogram → Input/Output
- Every diamond (decision) becomes an `if/else` or `switch`
- Loops in flowcharts become `while`, `do-while`, or `for`
- Good flowcharts lead to cleaner, more maintainable code

## Code Highlights

Main file: `src/day_16_flowchart_programming/main.cpp`
- All inputs from user
- Multiple flowchart implementations:
  - Simple eligibility checker
  - Grading system with multiple decisions
  - Number classification (positive/negative/zero)
  - Basic ATM withdrawal simulation
  - Finding maximum of three numbers

Test file: `tests/test_day_16.cpp`
- Tests core logic from the flowchart implementations

## Challenges / Mistakes Fixed

- Jumped straight to code without sketching logic → now starting with mental flowchart
- Deep nested ifs → improved structure with early returns
- Poor variable naming in complex decisions → used clear names

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 17)

Arrays and `std::vector`
- Fixed-size arrays
- Dynamic arrays using `std::vector`
- Basic array operations and algorithms

Thinking in flowcharts before coding is a professional habit — great step forward!