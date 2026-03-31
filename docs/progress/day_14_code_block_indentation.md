# Day 14 – Code Blocks and Indentation

**Date:** March 27, 2026  
**Status:** Completed

## Goals
- Understand what a code block is (`{}`) and why it matters
- Learn proper indentation styles and why consistency is critical
- Practice writing clean, readable code with correct block structure
- Explore common indentation mistakes and how to avoid them
- Develop good habits for maintainable, professional C++ code

## What I Learned / Key Takeaways

### Code Blocks (`{}`)
- Curly braces `{}` define a **scope** (code block)
- They group multiple statements together
- Even for single statements, braces are recommended (prevents bugs when adding code later)
- Every `if`, `else`, `for`, `while`, `do-while`, `try`, `catch`, function body uses blocks

### Indentation Rules (Professional Standard)
- Use **4 spaces** per indentation level (most common in modern C++)
- Never mix tabs and spaces
- Consistent indentation makes code readable and reduces bugs
- Proper alignment of braces (Allman vs K&R style — this project uses Allman style: opening brace on new line)

### Common Mistakes
- Forgetting braces after `if`/`for` → "dangling else" or logic bugs
- Inconsistent indentation → hard to read and maintain
- Deep nesting without proper formatting → "pyramid of doom"
- Missing closing brace → compilation error

## Code Highlights

Main file: `src/day_14_code_block_indentation/main.cpp`
- All inputs from user
- Multiple examples showing correct vs incorrect block structure
- Interactive programs with proper indentation and scoping

Test file: `tests/test_day_14.cpp`
- Focuses on logic that depends on correct block scoping

## Challenges / Mistakes Fixed

- Initially wrote "dangling else" example → fixed with proper braces
- Used inconsistent indentation → standardized to 4 spaces
- Deep nesting → refactored with early returns where appropriate

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 15)

While and Do-While loops
- Difference between `for`, `while`, and `do-while`
- When to choose each loop type
- Menu-driven programs using loops

Good indentation and clear code blocks are signs of a professional programmer. Well done!