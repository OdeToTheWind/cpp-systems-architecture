# placeholder
<!-- kpis:start -->
- **Curriculum completion:** 14 / 100 days covered, each with code, tests and a reflection.
- **Test cases:** 115 `TEST_CASE`s across 14 test executables.
- **Deliverables mapped to code:** 78 `DELIVERABLES` entries, each checked to resolve to a definition.
- **Source size:** 2,439 non-blank lines of C++ in `src/`.
- **Self-check questions:** 28 multiple-choice questions with explanations, plus 28 open bonus questions (14 hands-on, test-first tasks).
- **Coverage gate:** CI fails below ? % line coverage of `src/`.
- **Compilers in CI:** ?.
- **Operating systems in CI:** ?.
- **Quality checks per commit:** warnings as errors · clang-format · ASan + UBSan · unit tests with coverage · syllabus sync.
<!-- kpis:end -->
<!-- topic-map:start -->
| Phase | Days | What it covers |
|---|:-:|---|
| [1 · Beginner Fundamentals](syllabus.md#phase-1--beginner-fundamentals-days-124) | 1–24 | Write small, correct programs: types, strings, streams, arithmetic, conditionals, loops, functions, containers, pointers and error handling. |
| [2 · Intermediate C++](syllabus.md#phase-2--intermediate-c-days-2556) | 25–56 | Organise real programs: build tooling, classes and OOP, files and data formats, the STL, templates, RAII, persistence and deployment. |
| [3 · Networking, APIs & Automation](syllabus.md#phase-3--networking-apis--automation-days-5763) | 57–63 | Talk to the outside world: REST and JSON, HTTP messages, authentication, notifications, scraping and browser automation — all testable offline. |
| [4 · Advanced C++ Language & Tooling](syllabus.md#phase-4--advanced-c-language--tooling-days-6482) | 64–82 | Use the language at full strength: templates, concepts, ranges, ownership, move semantics, compile-time code, concurrency, coroutines, testing, packaging, profiling and storage engines. |
| [5 · Capstone-Style Systems Projects](syllabus.md#phase-5--capstone-style-systems-projects-days-83100) | 83–100 | Ship complete tools: CLIs, pipelines, services, plugins, validation, performance work, packaging and a portfolio capstone. |
<!-- topic-map:end -->
<!-- course-index:start -->
| Day | Topic | Level | Scenario you build | Links |
|---:|---|:-:|---|---|
| 1 | Variables, Types & Basic I/O | 🟢 | A *coding-club sign-up desk* that records each new member's profile card (name, age, height, membership tier, newsletter choice) and offers a mini calculator for splitting the club's membership fees. | [code](src/day_01_variables/lesson.hpp) · [tests](tests/test_day_01.cpp) · [notes](docs/progress/day-01-reflection.md) |
| 2 | String Manipulation | 🟢 | A *conference badge printer* that turns messy sign-up rows such as `" ada LOVELACE ; analytical engines ltd "` into clean, centred, fixed-width badges. | [code](src/day_02_strings/lesson.hpp) · [tests](tests/test_day_02.cpp) · [notes](docs/progress/day-02-reflection.md) |
| 3 | Input & Output Streams | 🟢 | A *workshop registration desk* that asks attendees questions in the console, re-asks after every invalid answer instead of crashing or looping forever, and prints a neatly aligned receipt with the ticket price. | [code](src/day_03_input_output/lesson.hpp) · [tests](tests/test_day_03.cpp) · [notes](docs/progress/day-03-reflection.md) |
| 4 | Variable Naming Rules | 🟢 | A *naming review bot* for pull requests: it inspects every proposed identifier and reports errors (the name will not compile), warnings about reserved names, and style advice so that variables, functions, types and constants each follow one convention. | [code](src/day_04_variable_names/lesson.hpp) · [tests](tests/test_day_04.cpp) · [notes](docs/progress/day-04-reflection.md) |
| 5 | Mathematical Operations | 🟢 | A *restaurant bill splitter* that works in whole cents: it applies a discount before tax, splits the total fairly between diners, and refuses calculations that would overflow or divide by zero. | [code](src/day_05_math_operations/lesson.hpp) · [tests](tests/test_day_05.cpp) · [notes](docs/progress/day-05-reflection.md) |
| 6 | Data Types & Fixed-width Integers | 🟢 | A *weather-station firmware memory planner*. The microcontroller has a few kilobytes of RAM, so every sensor field must use the smallest integer type that can hold its range – and the team must know exactly when a counter will wrap around. | [code](src/day_06_data_types/lesson.hpp) · [tests](tests/test_day_06.cpp) · [notes](docs/progress/day-06-reflection.md) |
| 7 | Type Conversion & Casting | 🟢 | A *payment-terminal message decoder*. Amounts arrive as raw integers and bytes from a card terminal; the decoder must convert them without silent truncation, talk to a legacy C checksum API, inspect the wire bytes, and tell payment messages from refunds. | [code](src/day_07_type_conversion/lesson.hpp) · [tests](tests/test_day_07.cpp) · [notes](docs/progress/day-07-reflection.md) |
| 8 | Conditional Statements | 🟢 | A *ski-resort lift operations board*. Every morning the duty manager enters the wind speed, temperature, visibility and fresh snow; the board decides which lifts may open, explains every closure, and rejects sensor readings that cannot be real. | [code](src/day_08_conditionals/lesson.hpp) · [tests](tests/test_day_08.cpp) · [notes](docs/progress/day-08-reflection.md) |
| 9 | Logical Operators | 🟢 | A *data-centre door controller* that decides whether a badge may open a door, explains every refusal, and proves with an evaluation trace exactly when C++ stops evaluating a condition. | [code](src/day_09_logical_operators/lesson.hpp) · [tests](tests/test_day_09.cpp) · [notes](docs/progress/day-09-reflection.md) |
| 10 | Randomisation | 🟢 | A *tabletop role-playing game master's toolkit*: dice notation such as `3d6+2`, loot that drops with a given chance, randomly generated non-player characters and a shuffled initiative order – all reproducible from a seed so a session can be replayed. | [code](src/day_10_randomisation/lesson.hpp) · [tests](tests/test_day_10.cpp) · [notes](docs/progress/day-10-reflection.md) |
| 11 | Error Handling | 🟢 | A *greenhouse sensor log reader*. Log files are messy, devices disappear and people type impossible values; the reader must keep going, count what went wrong and why, and only stop for errors it truly cannot handle. | [code](src/day_11_error_handling/lesson.hpp) · [tests](tests/test_day_11.cpp) · [notes](docs/progress/day-11-reflection.md) |
| 12 | Functions | 🟢 | A *bakery order counter*. Small, documented functions price each pastry by size, build up an order, apply a loyalty-card discount and print the receipt. This header holds only the declarations; their definitions live in lesson.cpp. | [code](src/day_12_functions/lesson.hpp) · [tests](tests/test_day_12.cpp) · [notes](docs/progress/day-12-reflection.md) |
| 13 | For Loops | 🟢 | A *marathon timing station*. Chip mats record each runner's split at every checkpoint; loops total the times, rank the finishers while skipping runners who did not finish, find the first runner under a target time, and print a pace chart. | [code](src/day_13_for_loops/lesson.hpp) · [tests](tests/test_day_13.cpp) · [notes](docs/progress/day-13-reflection.md) |
| 14 | Code Blocks and Indentation | 🟢 | A *snippet checker for a coding bootcamp*. Students paste C++ snippets; the checker finds unbalanced brackets with line numbers, flags `if`/`else`/loops without braces (the dangling-else trap), reports mixed or odd indentation, and re-indents the code. | [code](src/day_14_code_blocks/lesson.hpp) · [tests](tests/test_day_14.cpp) · [notes](docs/progress/day-14-reflection.md) |
| 15 | While and Do-While Loops | 🟢 | _planned_ | – |
| 16 | Flowchart Programming | 🟢 | _planned_ | – |
| 17 | Vectors and Maps | 🟢 | _planned_ | – |
| 18 | Positional and Named Arguments | 🟢 | _planned_ | – |
| 19 | Pointers and References | 🟢 | _planned_ | – |
| 20 | Returning Functions | 🟢 | _planned_ | – |
| 21 | Return vs Print | 🟢 | _planned_ | – |
| 22 | Documentation vs Comments | 🟢 | _planned_ | – |
| 23 | Scope, Lifetime & Global Variables | 🟢 | _planned_ | – |
| 24 | Debugging Techniques | 🟢 | _planned_ | – |
| 25 | Local Development Environment Setup | 🟡 | _planned_ | – |
| 26 | IDE Tips and Tricks | 🟡 | _planned_ | – |
| 27 | Object-Oriented Programming Basics | 🟡 | _planned_ | – |
| 28 | Creating Classes | 🟡 | _planned_ | – |
| 29 | Using External Libraries | 🟡 | _planned_ | – |
| 30 | Getters and Setters | 🟡 | _planned_ | – |
| 31 | Member Functions | 🟡 | _planned_ | – |
| 32 | Constructors and Initialiser Lists | 🟡 | _planned_ | – |
| 33 | Namespaces | 🟡 | _planned_ | – |
| 34 | Optional, Required & Default Parameters | 🟡 | _planned_ | – |
| 35 | Event Listeners & Callbacks | 🟡 | _planned_ | – |
| 36 | Instances and State | 🟡 | _planned_ | – |
| 37 | Graphics Programming | 🟡 | _planned_ | – |
| 38 | Game Development with OOP | 🟡 | _planned_ | – |
| 39 | Inheritance | 🟡 | _planned_ | – |
| 40 | Iterators | 🟡 | _planned_ | – |
| 41 | File I/O with fstream | 🟡 | _planned_ | – |
| 42 | Working with Directories | 🟡 | _planned_ | – |
| 43 | Reading and Writing CSV | 🟡 | _planned_ | – |
| 44 | Tabular Data Analysis | 🟡 | _planned_ | – |
| 45 | STL Algorithms | 🟡 | _planned_ | – |
| 46 | Variadic Templates | 🟡 | _planned_ | – |
| 47 | Desktop GUI Architecture | 🟡 | _planned_ | – |
| 48 | Static vs Dynamic Typing | 🟡 | _planned_ | – |
| 49 | Advanced Error Handling | 🟡 | _planned_ | – |
| 50 | Exception Safety & RAII | 🟡 | _planned_ | – |
| 51 | Working with JSON | 🟡 | _planned_ | – |
| 52 | Local Persistence | 🟡 | _planned_ | – |
| 53 | Sending Email (SMTP & MIME) | 🟡 | _planned_ | – |
| 54 | Date and Time with chrono | 🟡 | _planned_ | – |
| 55 | Hosting C++ Online | 🟡 | _planned_ | – |
| 56 | Command-Line Arguments | 🟡 | _planned_ | – |
| 57 | REST APIs & JSON | 🟠 | _planned_ | – |
| 58 | HTTP Requests | 🟠 | _planned_ | – |
| 59 | Query Parameters, Headers & Payloads | 🟠 | _planned_ | – |
| 60 | API Authentication | 🟠 | _planned_ | – |
| 61 | Notification Automation | 🟠 | _planned_ | – |
| 62 | Web Scraping | 🟠 | _planned_ | – |
| 63 | Browser Automation | 🟠 | _planned_ | – |
| 64 | Templates & Generic Programming | 🟠 | _planned_ | – |
| 65 | Concepts & Constraints | 🟠 | _planned_ | – |
| 66 | Ranges & Views | 🟠 | _planned_ | – |
| 67 | Smart Pointers & Ownership | 🟠 | _planned_ | – |
| 68 | Move Semantics | 🟠 | _planned_ | – |
| 69 | Operator Overloading | 🟠 | _planned_ | – |
| 70 | Compile-time Programming | 🟠 | _planned_ | – |
| 71 | Functional Tools | 🟠 | _planned_ | – |
| 72 | Design Patterns | 🟠 | _planned_ | – |
| 73 | Concurrency: Threads & Mutexes | 🟠 | _planned_ | – |
| 74 | Concurrency: Futures & Thread Pools | 🟠 | _planned_ | – |
| 75 | Coroutines | 🟠 | _planned_ | – |
| 76 | Atomics & Memory Order | 🟠 | _planned_ | – |
| 77 | Logging & Configuration | 🟠 | _planned_ | – |
| 78 | Unit Testing & Test Doubles | 🟠 | _planned_ | – |
| 79 | Build Systems & Packaging | 🟠 | _planned_ | – |
| 80 | Profiling & Performance | 🟠 | _planned_ | – |
| 81 | Regular Expressions | 🟠 | _planned_ | – |
| 82 | Building a Storage Engine | 🟠 | _planned_ | – |
| 83 | Robust CLI Application | 🔴 | _planned_ | – |
| 84 | Data Pipeline / ETL | 🔴 | _planned_ | – |
| 85 | Concurrent File Processor | 🔴 | _planned_ | – |
| 86 | Custom Logging & Monitoring Tool | 🔴 | _planned_ | – |
| 87 | Plugin-style Architecture | 🔴 | _planned_ | – |
| 88 | Automated Report Generator | 🔴 | _planned_ | – |
| 89 | Background Task Scheduler | 🔴 | _planned_ | – |
| 90 | Memory-efficient Large File Processor | 🔴 | _planned_ | – |
| 91 | Type-safe Configuration System | 🔴 | _planned_ | – |
| 92 | Test Suite for a Multi-module Library | 🔴 | _planned_ | – |
| 93 | Network Service Core | 🔴 | _planned_ | – |
| 94 | Data Validation & Cleaning Library | 🔴 | _planned_ | – |
| 95 | Performance-critical Module | 🔴 | _planned_ | – |
| 96 | Packaging a Real Tool | 🔴 | _planned_ | – |
| 97 | Automation Bot Suite | 🔴 | _planned_ | – |
| 98 | Scientific Simulation | 🔴 | _planned_ | – |
| 99 | Observability & Debugging Toolkit | 🔴 | _planned_ | – |
| 100 | Portfolio Capstone: Production-ready C++ Tool | 🔴 | _planned_ | – |
<!-- course-index:end -->
