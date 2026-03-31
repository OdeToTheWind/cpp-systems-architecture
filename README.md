# C++ Systems Architecture: 100-Day Engineering Challenge

[![C++ CI (MinGW / Ninja)](https://github.com/OdeToTheWind/cpp-systems-architecture/actions/workflows/cpp-ci.yml/badge.svg)](https://github.com/OdeToTheWind/cpp-systems-architecture/actions)
![C++ Standard](https://img.shields.io/badge/c++-17%20%7C%2020%20%7C%2023-blue)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Overview

This repository documents a structured **100-day professional development challenge** focused on modern C++ and systems programming. The goal is to build production-grade habits and deep understanding through:

- Modern C++ (C++17/20/23 features, strong typing, RAII, smart pointers, concepts)
- CMake-based build system with modular structure
- Unit testing (planned: Catch2 / GoogleTest) with high coverage
- Clean architecture, separation of concerns, and systems thinking
- Automated continuous integration via GitHub Actions (build + tests)
- Daily incremental progress with reflections and documentation

This project is part of a broader portfolio demonstrating disciplined engineering, performance awareness, and architectural maturity in C++.

## Project Structure

Organized for clarity, scalability, and professional workflows:

```text
cpp-systems-architecture/
├── .github/                # GitHub Actions CI pipelines
│   └── workflows/
│       └── cpp-ci.yml
├── build/                  # Build artifacts (ignored by git)
├── cmake/                  # Custom CMake modules & helpers
├── docs/                   # Architecture notes, memory layouts, design decisions
│   └── progress/
│       ├── day_01_memory.md
│       ├── day_02_strings.md
│       ├── day_03_prints.md
│       ├── day_04_variable_names.md
│       ├── day_05_maths_operations.md
│       ├── day_06_data_types.md
│       ├── day_07_convert_types_casting.md
│       ├── day_08_if_else_conditionals.md
│       ├── day_09_logical_operators.md 
│       ├── day_10_randomisation.md 
│       ├── day_11_error_handling.md 
│       ├── day_12_functions.md 
│       ├── day_13_for_loops.md 
│       ├── day_14_code_block_indentation.md 
│       ├── day_15_while_loops.md 
│       ├── day_16_flowchart_programming.md 
│       ├── day_17_maps_vectors.md 
│       └── day_XX_*.md      Flowchart Programming
├── include/                # Public headers (.hpp)
├── src/                    # Implementation code – one folder per day/topic
│   ├── day_01_memory/
│   │   └── main.cpp
│   ├── day_02_strings/
│   │   └── main.cpp
│   ├── day_03_prints/
│   │   └── main.cpp
│   ├── day_04_variable_names/
│   │   └── main.cpp
│   ├── day_05_maths_operations/
│   │   └── main.cpp
│   ├── day_06_data_types/
│   │   └── main.cpp
│   ├── day_07_convert_types_casting/
│   │   └── main.cpp
│   ├── day_08_if_else_conditionals/
│   │   └── main.cpp
│   ├── day_09_logical_operators/
│   │   └── main.cpp
│   ├── day_10_randomisation/
│   │   └── main.cpp
│   ├── day_11_error_handling/
│   │   └── main.cpp
│   ├── day_12_functions/
│   │   └── main.cpp
│   ├── day_13_for_loops/
│   │   └── main.cpp
│   ├── day_14_code_block_indentation/
│   │   └── main.cpp
│   ├── day_15_while_loops/
│   │   └── main.cpp
│   ├── day_16_flowchart_programming/
│   │   └── main.cpp
│   └── day_17_maps_vectors/   
│       └── main.cpp
├── tests/                  # Unit & integration tests       
│   ├── test_day_01.cpp
│   ├── test_day_02.cpp
│   ├── test_day_03.cpp
│   ├── test_day_04.cpp
│   ├── test_day_05.cpp
│   ├── test_day_06.cpp
│   ├── test_day_07.cpp
│   ├── test_day_08.cpp
│   ├── test_day_09.cpp
│   ├── test_day_10.cpp
│   ├── test_day_11.cpp
│   ├── test_day_12.cpp
│   ├── test_day_13.cpp
│   ├── test_day_14.cpp
│   ├── test_day_15.cpp
│   ├── test_day_16.cpp
│   └── test_day_17.cpp
├── CMakeLists.txt          # Root build configuration
├── procpp.sh               # Convenience script to build/run
├── README.md
└── .gitignore
```
### Key Engineering Practices

- **Modern C++** — C++17 as baseline, moving toward C++20/23 (concepts, ranges, modules where supported)
- **Build System** — CMake 3.20+, Ninja generator preferred, reproducible builds
- **Testing** — Incremental unit tests per day (Catch2 or GoogleTest planned)
- **CI/CD** — GitHub Actions workflow for build verification, compiler warnings, static analysis (planned: clang-tidy)
- **Code Quality** — Strict warnings (-Wall -Wextra -Wpedantic -Wshadow -Wconversion), zero UB focus
- **Systems Thinking** — Emphasis on memory layout, pointers/references, performance, concurrency later


### Daily Progress

| Day | Topic | Status | Key Focus / Deliverables |
|-----|-------|--------|--------------------------|
| 01 | Variables, Types & Basic I/O | ✅ Completed | Fundamental types, initialization, cin/cout/getline, mini calculator |
| 02 | String Manipulation (std::string) | ✅ Completed | length/size, substr, find, concatenation, formatting, input cleaning |
| 03 | Input and Print Functions (cin / cout) | ✅ Completed | Console I/O patterns, buffer cleaning, getline, conditional formatting |
| 04 | Variable Naming Rules & Conventions | ✅ Completed | Naming rules, snake_case/camelCase, intention-revealing names, anti-patterns |
| 05 | Mathematical Operations in C++ | ✅ Completed | Arithmetic operators, compound assignment, increment/decrement, <cmath> functions |
| 06 | Data Types in C++ | ✅ Completed | Fundamental types, modifiers, sizes/ranges, type aliases |
| 07 | Type Conversion & Casting | ✅ Completed | Implicit conversions, C-style vs modern casts, static_cast, dynamic_cast, const_cast, reinterpret_cast |
| 08 | Conditional Statements (if / else) | ✅ Completed | if/else if chains, early input validation, keeping nesting shallow |
| 09 | Logical Operators | ✅ Completed | (&&, 		, !) |
| 10 | Randomization (<random>) | ⏳ Planned | std::mt19937 engine, std::random_device, uniform distributions vs legacy rand() |

Detailed reflections, code explanations, memory diagrams, and lessons learned are in  [docs/progress/](./docs/progress/)

### Getting Started
## Prerequisites

- C++ compiler (GCC 12+, Clang 15+, MSVC 2022+ recommended)
- CMake 3.20+
- Ninja (preferred) or Make
- Git

## Setup (MSYS2 / MinGW / UCRT64 recommended on Windows)
# Clone the repository
```bash
git clone https://github.com/OdeToTheWind/cpp-systems-architecture.git
cd cpp-systems-architecture

# (Optional but recommended) Update MSYS2 packages
pacman -Syu

# Install toolchain if not already present
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```
## Build & Run
```bash
# Build everything (uses Ninja if available)
./procpp.sh

# Run Day 1 example
./build/day_01_memory.exe

# Or from inside build folder
cd build && ./day_01_memory.exe
```
## Future Commands (planned)
```bash
Bash# Run tests (once added)
ctest -V

# Static analysis (future)
clang-tidy ...
```
### Contributing
This is a personal challenge repository. Issues, suggestions, and constructive discussions are welcome.

### License
MIT License – see the LICENSE file for details.

<!--

### Quick Notes on Improvements & Choices

- Adapted tone, badges, and structure directly from your Python README
- Used realistic C++ badges (C++ standard range, license)
- CI badge links to your actual workflow file (`cpp-ci.yml`)
- Emphasized systems architecture angle (memory, pointers, performance) to match repo name
- Kept setup instructions Windows/MSYS2-focused (since that's your environment), but still general
- Progress table mirrors your Python version (easy to update)
- No emojis in headings, professional phrasing
- Left room for future growth (testing, clang-tidy, modules, sanitizers)

You can copy-paste this directly into your `README.md`.

If you want:
- A real LICENSE file (MIT boilerplate)
- Badges for code coverage, clang-tidy, or build status on more platforms
- Expansion of the progress table with more planned days
- Addition of planned tools (vcpkg, Conan, sanitizers)

Just let me know — happy to refine further.

Good luck with Day 2 and the rest of the challenge! Keep the momentum.

-->
