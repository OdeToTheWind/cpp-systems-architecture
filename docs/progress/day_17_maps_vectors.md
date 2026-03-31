# Day 17 – Vectors and Maps in C++ (Lists & Dictionaries)

**Date:** March 30, 2026  
**Status:** Completed

## Goals
- Master `std::vector` (dynamic array / list)
- Master `std::map` (ordered dictionary / key-value store)
- Learn basic operations: add, access, modify, remove, iterate
- Understand when to use vector vs map
- Practice real-world usage with full user input

## What I Learned / Key Takeaways

### `std::vector` – Dynamic List
- Resizable array
- Fast random access (`O(1)`)
- Use when order matters and you need fast indexing
- Common operations: `push_back()`, `size()`, `[]`, `at()`, `clear()`

### `std::map` – Ordered Dictionary
- Key-value pairs (keys are unique and sorted)
- Slower than vector but provides fast lookup by key (`O(log n)`)
- Use when you need to associate data with a unique key (like a real dictionary)

### Important Notes
- Prefer `std::vector` for most sequential data
- Prefer `std::map` when you need fast lookup by meaningful key
- Always check bounds with `.at()` in learning phase (throws exception)
- Range-based for loops work beautifully with both

## Code Highlights

Main file: `src/day_17_maps_vectors/main.cpp`
- All data entered by user
- Student score management using `std::map<std::string, double>`
- Dynamic shopping list using `std::vector`
- Interactive menu to add, view, search, and remove items

Test file: `tests/test_day_17.cpp`
- Tests vector and map operations

## Challenges / Mistakes Fixed

- Used fixed arrays initially → switched to `std::vector`
- Manual indexing errors → used range-based loops
- Tried to use map with duplicate keys → understood uniqueness
- Poor user experience → added clear menu and validation

## CMake & Build Notes

- Folder auto-detected by CMakeLists.txt
- Built & run via `./procpp.sh`

## Next Day Preview (Day 18)

More STL containers + algorithms
- `std::unordered_map` (faster lookup)
- Basic STL algorithms (`std::sort`, `std::find`, etc.)
- Combining containers effectively

Mastering `vector` and `map` is a major milestone — you can now handle real data structures!