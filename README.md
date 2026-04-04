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
│       ├── day_18_positional_keyword_arguments.md  
│       ├── day_19_pointer_references.md   
│       ├── day_20_returning_functions.md  
│       ├── day_21_return_vs_print.md  
│       ├── day_22_docs_strings_comments.md  
│       ├── day_23_scope_local_global_variables.md  
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
│   ├── day_17_maps_vectors/
│   │   └── main.cpp
│   ├── day_18_positional_keyword_arguments/
│   │   └── main.cpp
│   ├── day_19_pointer_references/
│   │   └── main.cpp
│   ├── day_20_returning_functions/
│   │   └── main.cpp
│   ├── day_21_return_vs_print/
│   │   └── main.cpp
│   ├── day_22_docs_strings_comments/
│   │   └── main.cpp
│   └── day_23_scope_local_global_variables/       Return vs. Print || Doc Strings vs. Comments (Doxygen style) || Scope and Local/Global Variables
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
│   ├── test_day_17.cpp
│   ├── test_day_18.cpp
│   ├── test_day_19.cpp
│   ├── test_day_20.cpp
│   ├── test_day_21.cpp
│   ├── test_day_22.cpp
│   └── test_day_23.cpp
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
| 01 | Variables, Types & Basic I/O | ✅ Completed | Fundamental types, initialization, `cin`/`cout`/`getline`, mini calculator |
| 02 | String Manipulation (`std::string`) | ✅ Completed | `length`, `substr`, `find`, concatenation, formatting, input cleaning |
| 03 | Input and Print Functions (`cin`/`cout`) | ✅ Completed | Console I/O patterns, buffer cleaning, formatted output |
| 04 | Variable Naming Rules & Conventions | ✅ Completed | snake_case, intention-revealing names, anti-patterns |
| 05 | Mathematical Operations in C++ | ✅ Completed | Arithmetic operators, compound assignment, `<cmath>` functions |
| 06 | Data Types in C++ | ✅ Completed | Fundamental types, modifiers, sizes/ranges, fixed-width integers |
| 07 | Type Conversion & Casting | ✅ Completed | Implicit vs explicit, `static_cast`, `dynamic_cast`, narrowing |
| 08 | Conditional Statements (`if`/`else if`/`else`) | ✅ Completed | Decision making, input validation, early returns |
| 09 | Logical Operators (`&&`, `\|\|`, `!`) | ✅ Completed | Combining conditions, short-circuiting, De Morgan's laws |
| 10 | Randomisation (`<random>`) | ✅ Completed | `std::mt19937`, uniform distributions, modern RNG |
| 11 | Error Handling (`try`/`catch`/`throw`) | ✅ Completed | Exception handling, `std::runtime_error`, input validation |
| 12 | Functions | ✅ Completed | Declaration, parameters, return values, overloading |
| 13 | For Loops | ✅ Completed | Traditional and range-based for loops, nested loops |
| 14 | Code Blocks and Indentation | ✅ Completed | Scoping, consistent style, readability best practices |
| 15 | While and Do-While Loops | ✅ Completed | Pre-test vs post-test loops, menu systems |
| 16 | Flowchart Programming | ✅ Completed | Translating logic flow into clean C++ code |
| 17 | Vectors and Maps | ✅ Completed | Dynamic lists (`std::vector`) and dictionaries (`std::map`) |
| 18 | Positional & Keyword Arguments | ✅ Completed | Default arguments, function overloading, Named Parameter Idiom |
| 19 | Pointers and References | ✅ Completed | Raw pointers, references, pass-by-value vs pass-by-reference |
| 20 | Returning Functions (`std::function`) | ✅ Completed | Function pointers, `std::function`, callbacks |
| 21 | Return vs Print | ✅ Completed | Function design, side effects vs pure functions |
| 22 | Documentation (Doxygen style) | ✅ Completed | Comments vs docstrings, generating documentation |
| 23 | Scope and Local/Global Variables | ✅ Completed | Variable lifetime, shadowing, namespace usage |
| 24 | Debugging Techniques | ⏳ Planned | GDB basics, debugging strategies |

### Intermediate Projects (Section 2)

| Day | Topic | Status | Key Focus / Deliverables |
|-----|-------|--------|--------------------------|
| 25 | Local Development Environment Setup | ⏳ Planned | IDE/Compiler setup, CMake best practices |
| 26 | IDE Tips and Tricks | ⏳ Planned | Advanced editor features, debugging in IDE |
| 27 | Object-Oriented Programming Basics | ⏳ Planned | OOP principles in C++ |
| 28 | Creating Classes | ⏳ Planned | Class definition, encapsulation |
| 29 | Using External Libraries | ⏳ Planned | Header-only vs linking, vcpkg/Conan |
| 30 | Getters and Setters | ⏳ Planned | Accessors and mutators |
| 31 | C++ Methods | ⏳ Planned | Member functions, const methods |
| 32 | Constructors and Initializers | ⏳ Planned | Default, parameterized, copy constructors |
| 33 | Namespaces | ⏳ Planned | Namespace aliasing and organization |
| 34 | Optional, Required & Default Parameters | ⏳ Planned | Advanced parameter handling |
| 35 | Event Listeners & Callbacks | ⏳ Planned | Observer pattern basics |
| 36 | Instances and State | ⏳ Planned | Object state management |
| 37 | Graphics with SFML or Raylib | ⏳ Planned | Basic graphics programming |
| 38 | Game Development with OOP | ⏳ Planned | Simple game architecture |
| 39 | Inheritance | ⏳ Planned | Base and derived classes |
| 40 | Iterators | ⏳ Planned | STL iterators and ranges |
| 41 | File I/O (`fstream`) | ⏳ Planned | Reading/writing text and binary files |
| 42 | Working with Directories | ⏳ Planned | Filesystem library (`std::filesystem`) |
| 43 | Reading/Writing CSV | ⏳ Planned | CSV parsing and generation |
| 44 | Data Frameworks (Eigen / Dlib) | ⏳ Planned | Introduction to numerical libraries |
| 45 | Vector/Map Transformations | ⏳ Planned | STL algorithms on containers |
| 46 | Variadic Templates | ⏳ Planned | Packing/unpacking arguments |
| 47 | Desktop GUI with Qt / wxWidgets | ⏳ Planned | Basic GUI application |
| 48 | Static vs Dynamic Typing | ⏳ Planned | Type safety in C++ |
| 49 | Advanced Error Handling | ⏳ Planned | Custom exceptions, error codes |
| 50 | Try / Catch / Throw Deep Dive | ⏳ Planned | Exception safety, RAII |
| 51 | Working with JSON (`nlohmann/json`) | ⏳ Planned | JSON serialization/deserialization |
| 52 | Local Persistence | ⏳ Planned | Saving/loading application state |
| 53 | Sending Email (libcurl) | ⏳ Planned | Network integration |
| 54 | Date and Time (`<chrono>`) | ⏳ Planned | Modern time handling |
| 55 | Hosting C++ Online (WebAssembly / CGI) | ⏳ Planned | Deploying C++ code |
| 56 | APIs & HTTP Requests | ⏳ Planned | Making HTTP calls |
| 57 | Sending Parameters with Requests | ⏳ Planned | REST API consumption |
| 58 | APIs with Authentication | ⏳ Planned | Token-based authentication |
| 59 | Web Scraping | ⏳ Planned | Parsing HTML with Gumbo |
| 60 | Browser Automation | ⏳ Planned | WebDriver bindings |
| 61 | Web Development with Crow / Pistache | ⏳ Planned | Lightweight web frameworks |
| 62 | Command Line Tools | ⏳ Planned | Building CLI applications |
| 63 | C++ Templates & Design Patterns | ⏳ Planned | Generic programming |
| 64 | Advanced STL & Algorithms | ⏳ Planned | `std::sort`, `std::find`, ranges |

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
