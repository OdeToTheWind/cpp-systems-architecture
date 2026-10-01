# placeholder
<!-- kpis:start -->
- **Curriculum completion:** 32 / 100 days covered, each with code, tests and a reflection.
- **Test cases:** 250 `TEST_CASE`s across 32 test executables.
- **Deliverables mapped to code:** 182 `DELIVERABLES` entries, each checked to resolve to a definition.
- **Source size:** 5,535 non-blank lines of C++ in `src/`.
- **Self-check questions:** 64 multiple-choice questions with explanations, plus 64 open bonus questions (32 hands-on, test-first tasks).
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
| 15 | While and Do-While Loops | 🟢 | A *vending-machine controller*. It accepts coins until the price is covered (or the customer types `cancel`), pays change with as few coins as possible, locks the service panel after three wrong PINs, and shows its menu at least once per session. | [code](src/day_15_while_loops/lesson.hpp) · [tests](tests/test_day_15.cpp) · [notes](docs/progress/day-15-reflection.md) |
| 16 | Flowchart Programming | 🟢 | An *airport check-in kiosk*. Each rule – the baggage fee, the boarding check and the queue the kiosk works through – is first drawn as a flowchart (kept in the code as Mermaid text) and then translated shape by shape into structured C++. | [code](src/day_16_flowchart_programming/lesson.hpp) · [tests](tests/test_day_16.cpp) · [notes](docs/progress/day-16-reflection.md) |
| 17 | Vectors and Maps | 🟢 | A *community tool library*. A `std::map` keeps the stock of every tool by name, a `std::vector` keeps the waiting list in arrival order, and borrow counts are sorted into a "most popular tools" board. | [code](src/day_17_vectors_maps/lesson.hpp) · [tests](tests/test_day_17.cpp) · [notes](docs/progress/day-17-reflection.md) |
| 18 | Positional and Named Arguments | 🟢 | An *airline booking API*. The route is passed positionally (it is always needed), optional extras have defaults, overloads accept a date in two shapes, and the many optional settings travel in a parameter struct filled with C++20 designated initialisers – C++'s closest equivalent to keyword arguments. | [code](src/day_18_named_arguments/lesson.hpp) · [tests](tests/test_day_18.cpp) · [notes](docs/progress/day-18-reflection.md) |
| 19 | Pointers and References | 🟢 | A *hospital ward bed board*. Each bed is a slot in a fixed array; the board hands out pointers to free beds (or nullptr when the ward is full), moves patients between beds through references, and only ever reads through const pointers when it reports. | [code](src/day_19_pointers_references/lesson.hpp) · [tests](tests/test_day_19.cpp) · [notes](docs/progress/day-19-reflection.md) |
| 20 | Returning Functions | 🟢 | A *shipping-rate engine for an online shop*. Carrier rates are plain functions looked up through function pointers, promotions are lambdas built at run time and composed into one pricing rule, quotes come back as structs, and the checkout page receives each quote through a callback. | [code](src/day_20_returning_functions/lesson.hpp) · [tests](tests/test_day_20.cpp) · [notes](docs/progress/day-20-reflection.md) |
| 21 | Return vs Print | 🟢 | An *apartment-building electricity billing tool*. The same tiered tariff is written twice: once as a function that prints as it calculates (hard to reuse or test), and once as pure functions that return a bill which is formatted separately – and only the second design can total the whole building. | [code](src/day_21_return_vs_print/lesson.hpp) · [tests](tests/test_day_21.cpp) · [notes](docs/progress/day-21-reflection.md) |
| 22 | Documentation vs Comments | 🟢 | A *kitchen unit-conversion library* that is documented the professional way – Doxygen comments describe what each function promises, ordinary comments explain why – plus a small documentation auditor that checks a header for undocumented functions. | [code](src/day_22_documentation/lesson.hpp) · [tests](tests/test_day_22.cpp) · [notes](docs/progress/day-22-reflection.md) |
| 23 | Scope, Lifetime & Global Variables | 🟢 | A *deli-counter ticket dispenser*. Shop-wide settings live in a namespace, the ticket counter is a function-local static that survives between calls, helper code is hidden with internal linkage, and a lifetime log shows exactly when each object is born and destroyed. | [code](src/day_23_scope_lifetime/lesson.hpp) · [tests](tests/test_day_23.cpp) · [notes](docs/progress/day-23-reflection.md) |
| 24 | Debugging Techniques | 🟢 | A *library late-fee calculator* that shipped with a real bug: some borrowers were charged for the grace day. The bug is reproduced with a minimal failing case, traced with debug output on std::cerr, cornered by bisecting the inputs, and fixed – with assertions guarding the invariants so it cannot come back. | [code](src/day_24_debugging/lesson.hpp) · [tests](tests/test_day_24.cpp) · [notes](docs/progress/day-24-reflection.md) |
| 25 | Local Development Environment Setup | 🟡 | A *project doctor* that examines a C++ checkout – this repository by default – and reports whether the local setup follows best practice: a CMake build, presets for development and sanitizers, strict warnings, a formatter configuration, an ignored build folder, and a modern compiler and language standard. | [code](src/day_25_dev_environment/lesson.hpp) · [tests](tests/test_day_25.cpp) · [notes](docs/progress/day-25-reflection.md) |
| 26 | IDE Tips and Tricks | 🟡 | A *pocket IDE coach*: a searchable shortcut cheat-sheet for VS Code, CLion and Visual Studio on each operating system, a live-template expander, and the refactorings an IDE performs – find references, go to definition and a token-aware rename that never touches strings, comments or longer names that merely contain the old one. | [code](src/day_26_ide_tips/lesson.hpp) · [tests](tests/test_day_26.cpp) · [notes](docs/progress/day-26-reflection.md) |
| 27 | Object-Oriented Programming Basics | 🟡 | A *museum ticket kiosk payment gateway*. Cards, prepaid wallets and gift vouchers are very different, yet the kiosk charges all of them through one abstract interface – and a card number never leaves its object except as `**** 1111`. | [code](src/day_27_oop_basics/lesson.hpp) · [tests](tests/test_day_27.cpp) · [notes](docs/progress/day-27-reflection.md) |
| 28 | Creating Classes | 🟡 | A *gym class booking system*. A `FitnessClass` object guards its own rules – never more attendees than places, nobody booked twice, the waiting list promoted in order – so no caller can ever put it into an invalid state. | [code](src/day_28_classes/lesson.hpp) · [tests](tests/test_day_28.cpp) · [notes](docs/progress/day-28-reflection.md) |
| 29 | Using External Libraries | 🟡 | A *newsletter editor's text toolkit* built on a library called textkit, which ships in two flavours exactly like real dependencies do: a header-only part (just include it) and a compiled static library (CMake target `textkit`, linked by this lesson). A dependency checker reads a manifest, compares semantic versions and prints the CMake you would write to get each library with find_package or FetchContent. | [code](src/day_29_external_libraries/lesson.hpp) · [tests](tests/test_day_29.cpp) · [notes](docs/progress/day-29-reflection.md) |
| 30 | Getters and Setters | 🟡 | An *aquarium controller*. The keeper can read and set the water temperature in Celsius or Fahrenheit and adjust the pH and lighting, but the controller refuses any value that would harm the fish – and stores temperatures in one exact internal unit. | [code](src/day_30_getters_setters/lesson.hpp) · [tests](tests/test_day_30.cpp) · [notes](docs/progress/day-30-reflection.md) |
| 31 | Member Functions | 🟡 | A *coffee-roastery batch tracker*. Each roast batch has methods that change it (and return `*this` so they chain), const methods that only read it, and static members that belong to the roastery as a whole: the batch-number counter and the list of beans it buys. | [code](src/day_31_member_functions/lesson.hpp) · [tests](tests/test_day_31.cpp) · [notes](docs/progress/day-31-reflection.md) |
| 32 | Constructors and Initialiser Lists | 🟡 | A *car-rental reservation desk*. Every way of creating a reservation – a blank walk-in form, a full booking, a booking for "N days from a start day", or a copy of last year's booking – goes through a constructor that guarantees a valid object from the very first moment it exists. | [code](src/day_32_constructors/lesson.hpp) · [tests](tests/test_day_32.cpp) · [notes](docs/progress/day-32-reflection.md) |
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
