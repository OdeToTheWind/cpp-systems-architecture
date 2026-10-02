# C++ Systems Architecture — Syllabus

The complete 100-day curriculum: five phases, each building on the one before.
Every day is a self-contained mini-project with its own code, tests and reflection.

## How to read this syllabus

| Column | Meaning |
|---|---|
| **Day** | Position in the course; `src/day_XX_<topic>/` holds its code. |
| **Topic** | What the day teaches. |
| **Key Learnings / Deliverables** | The skills the day's code must demonstrate. Each one maps to real code via the lesson's `DELIVERABLES` table. |
| **Level** | Beginner · Intermediate · Advanced · Capstone. |
| **Status** | `Covered` (code, tests and reflection exist and agree) or `Planned`. |

> **How status is verified.** A day is marked **Covered** only when it has
> `src/day_XX_<topic>/lesson.hpp` with a `DELIVERABLES` table whose every symbol is
> defined in that day's code, `tests/test_day_XX.cpp` that includes and exercises that
> code with at least five test cases, and `docs/progress/day-XX-reflection.md` generated
> from the same facts. `tests/tooling/test_syllabus_sync.py` enforces this in CI, so these
> tables cannot drift from the code.

## Phase overview

| Phase | Days | Focus |
|---|---|---|
| 1 · Beginner Fundamentals | 1–24 | Write small, correct programs: types, strings, streams, arithmetic, conditionals, loops, functions, containers, pointers and error handling. |
| 2 · Intermediate C++ | 25–56 | Organise real programs: build tooling, classes and OOP, files and data formats, the STL, templates, RAII, persistence and deployment. |
| 3 · Networking, APIs & Automation | 57–63 | Talk to the outside world: REST and JSON, HTTP messages, authentication, notifications, scraping and browser automation — all testable offline. |
| 4 · Advanced C++ Language & Tooling | 64–82 | Use the language at full strength: templates, concepts, ranges, ownership, move semantics, compile-time code, concurrency, coroutines, testing, packaging, profiling and storage engines. |
| 5 · Capstone-Style Systems Projects | 83–100 | Ship complete tools: CLIs, pipelines, services, plugins, validation, performance work, packaging and a portfolio capstone. |

## Phase 1 · Beginner Fundamentals (Days 1–24)

**By the end of this phase you can** write correct, warning-free C++20 that validates its input, never invokes undefined behaviour on bad data, and is covered by unit tests.

| Day | Topic | Key Learnings / Deliverables | Level | Status |
|----:|-------|------------------------------|-------|--------|
| 01 | Variables, Types & Basic I/O | Fundamental types, brace initialisation, auto, const, reading values with std::cin and std::getline, a mini calculator | Beginner | Covered |
| 02 | String Manipulation | std::string length, find, substr, concatenation, trimming and case-folding input, aligned formatting | Beginner | Covered |
| 03 | Input & Output Streams | Validated stream extraction, recovering from failed reads, buffer clearing, iomanip formatting | Beginner | Covered |
| 04 | Variable Naming Rules | Legal identifiers, reserved words and reserved names, snake_case and PascalCase conventions, intention-revealing names | Beginner | Covered |
| 05 | Mathematical Operations | Arithmetic and compound assignment, precedence, integer vs floating division, overflow-safe arithmetic, cmath functions | Beginner | Covered |
| 06 | Data Types & Fixed-width Integers | Fundamental types and modifiers, sizeof and numeric_limits, fixed-width integers, signed vs unsigned wrap-around | Beginner | Covered |
| 07 | Type Conversion & Casting | Implicit promotions, static_cast, const_cast, reinterpret_cast, dynamic_cast, narrowing checks with brace initialisation | Beginner | Covered |
| 08 | Conditional Statements | if / else if / else chains, switch with enums, guard clauses, the conditional operator | Beginner | Covered |
| 09 | Logical Operators | &&, \|\| and !, short-circuit evaluation, De Morgan's laws, named boolean conditions | Beginner | Covered |
| 10 | Randomisation | std::mt19937 engines, seeding, uniform, normal and bernoulli distributions, shuffling, reproducible randomness | Beginner | Covered |
| 11 | Error Handling | throw, try and catch, the std::exception hierarchy, custom exceptions, rethrowing, input validation | Beginner | Covered |
| 12 | Functions | Declarations vs definitions, parameters and return values, pass by value and const reference, overloading, default arguments | Beginner | Covered |
| 13 | For Loops | Counted and range-based for loops, nested loops, break and continue, index-safe iteration | Beginner | Covered |
| 14 | Code Blocks and Indentation | Block scope, braces for every branch, the dangling-else trap, consistent indentation style | Beginner | Covered |
| 15 | While and Do-While Loops | Pre-test and post-test loops, sentinel loops, menu loops, termination on end of input | Beginner | Covered |
| 16 | Flowchart Programming | Translating decisions, processes and loops from a flowchart into structured C++ | Beginner | Covered |
| 17 | Vectors and Maps | std::vector and std::map insert, lookup, update and erase, iteration, choosing the right container | Beginner | Covered |
| 18 | Positional and Named Arguments | Positional parameters, default arguments, overloads, parameter structs and designated initialisers | Beginner | Covered |
| 19 | Pointers and References | Address-of and dereference, references, pass by pointer vs reference, nullptr checks, const correctness | Beginner | Covered |
| 20 | Returning Functions | Returning values and structs, function pointers, std::function, lambdas returned from functions, callbacks | Beginner | Covered |
| 21 | Return vs Print | Pure functions vs side effects, returning data and formatting it separately, testable design | Beginner | Covered |
| 22 | Documentation vs Comments | Doxygen comments, why-comments vs what-comments, documenting preconditions and errors | Beginner | Covered |
| 23 | Scope, Lifetime & Global Variables | Block, function, namespace and static scope, shadowing, object lifetime, internal linkage | Beginner | Covered |
| 24 | Debugging Techniques | Reproducing bugs, assertions, std::cerr tracing, bisecting failing inputs, debugger-friendly code | Beginner | Covered |

## Phase 2 · Intermediate C++ (Days 25–56)

**By the end of this phase you can** design classes and modules, manage resources with RAII, read and write files and data formats safely, and use the STL fluently.

| Day | Topic | Key Learnings / Deliverables | Level | Status |
|----:|-------|------------------------------|-------|--------|
| 25 | Local Development Environment Setup | Compilers, CMake presets, warnings as errors, sanitizers, a reproducible project layout | Intermediate | Covered |
| 26 | IDE Tips and Tricks | Navigation and refactoring workflows, keyboard shortcuts, token-aware rename, code templates | Intermediate | Covered |
| 27 | Object-Oriented Programming Basics | Encapsulation, abstraction with pure virtual interfaces, polymorphism, information hiding | Intermediate | Covered |
| 28 | Creating Classes | Class definitions, constructors, member functions, invariants, explicit constructors, operator<< | Intermediate | Covered |
| 29 | Using External Libraries | Header-only vs compiled libraries, linking a static library, find_package and FetchContent, version checks | Intermediate | Covered |
| 30 | Getters and Setters | Accessors and mutators, validation in setters, unit conversion behind an interface | Intermediate | Covered |
| 31 | Member Functions | Const member functions, static member functions, static data, this, method chaining | Intermediate | Covered |
| 32 | Constructors and Initialiser Lists | Default, parameterised, delegating and copy constructors, member initialiser lists, validation at construction | Intermediate | Covered |
| 33 | Namespaces | Nested and inline namespaces, namespace aliases, anonymous namespaces, using-declarations vs using-directives | Intermediate | Covered |
| 34 | Optional, Required & Default Parameters | std::optional parameters, overload sets, builder objects, parameter ordering rules | Intermediate | Covered |
| 35 | Event Listeners & Callbacks | Observer pattern, subscription handles, unsubscribing safely, std::function listeners | Intermediate | Covered |
| 36 | Instances and State | Per-object state, state machines with enum class, transition validation, lifecycle history | Intermediate | Covered |
| 37 | Graphics Programming | Raster canvas, Bresenham lines, circles and fills, exporting PPM images, separating drawing from display | Intermediate | Covered |
| 38 | Game Development with OOP | Game entities as classes, turn-based game loop, injected randomness, win and lose conditions | Intermediate | Covered |
| 39 | Inheritance | Base and derived classes, virtual and override, final, virtual destructors, multiple inheritance of interfaces | Intermediate | Covered |
| 40 | Iterators | Iterator categories, begin and end, writing a custom iterator, iterator invalidation | Intermediate | Covered |
| 41 | File I/O with fstream | ifstream and ofstream, text and binary files, append mode, error checking, RAII file handling | Intermediate | Covered |
| 42 | Working with Directories | std::filesystem paths, creating and walking directories, filtering by extension, sandboxed operations | Intermediate | Covered |
| 43 | Reading and Writing CSV | Parsing quoted CSV fields, validating rows, reporting bad rows, writing CSV safely | Intermediate | Covered |
| 44 | Tabular Data Analysis | Column-oriented tables, filtering, derived columns, group-by aggregation, summary statistics | Intermediate | Covered |
| 45 | STL Algorithms | transform, copy_if, accumulate, sort with custom comparators, partition, erase-remove | Intermediate | Covered |
| 46 | Variadic Templates | Parameter packs, fold expressions, perfect forwarding, std::tuple and std::apply | Intermediate | Covered |
| 47 | Desktop GUI Architecture | Model-View-Presenter, widget-free presenters, input validation, headless testing of UI logic | Intermediate | Covered |
| 48 | Static vs Dynamic Typing | Static type checking, std::variant and std::visit, std::any, type-safe heterogeneous data | Intermediate | Covered |
| 49 | Advanced Error Handling | Custom exception hierarchies, std::error_code, result types, choosing exceptions vs error values | Intermediate | Covered |
| 50 | Exception Safety & RAII | Basic, strong and nothrow guarantees, scope guards, copy-and-swap, noexcept | Intermediate | Covered |
| 51 | Working with JSON | JSON value model, recursive-descent parsing, serialisation with escaping, error positions | Intermediate | Covered |
| 52 | Local Persistence | Saving and loading application state, atomic writes, schema versions, recovering from corrupt files | Intermediate | Covered |
| 53 | Sending Email (SMTP & MIME) | Building MIME messages, address validation, the SMTP dialogue over an injectable transport, dry runs | Intermediate | Covered |
| 54 | Date and Time with chrono | Durations and time points, calendar dates, business-day arithmetic, UTC offsets | Intermediate | Covered |
| 55 | Hosting C++ Online | Request/response handlers, CGI-style environment parsing, routing, deployment-ready configuration | Intermediate | Covered |
| 56 | Command-Line Arguments | argc and argv, flags and options, positional arguments, usage messages and exit codes | Intermediate | Covered |

## Phase 3 · Networking, APIs & Automation (Days 57–63)

**By the end of this phase you can** build clients that speak HTTP and JSON correctly, keep secrets out of code, and test every network interaction without a network.

| Day | Topic | Key Learnings / Deliverables | Level | Status |
|----:|-------|------------------------------|-------|--------|
| 57 | REST APIs & JSON | HTTP methods and their meaning, status codes, resource routing, JSON request and response bodies | Advanced | Covered |
| 58 | HTTP Requests | HTTP/1.1 request and response formats, parsing status lines and headers, timeouts and retries over an injectable transport | Advanced | Covered |
| 59 | Query Parameters, Headers & Payloads | Percent-encoding, query strings, custom headers, form and JSON request bodies | Advanced | Covered |
| 60 | API Authentication | API keys, Bearer tokens, Basic auth with Base64, secrets from environment variables, redaction | Advanced | Covered |
| 61 | Notification Automation | Health checks, alert thresholds, webhook payloads, rate limiting and dry-run delivery | Advanced | Covered |
| 62 | Web Scraping | Tokenising HTML, extracting elements and attributes, robots.txt rules, polite crawling | Advanced | Covered |
| 63 | Browser Automation | WebDriver commands, locator strategies, explicit waits, the page-object pattern | Advanced | Covered |

## Phase 4 · Advanced C++ Language & Tooling (Days 64–82)

**By the end of this phase you can** write generic, constrained, move-aware code, pick the right concurrency tool, measure before optimising, and package a library.

| Day | Topic | Key Learnings / Deliverables | Level | Status |
|----:|-------|------------------------------|-------|--------|
| 64 | Templates & Generic Programming | Function and class templates, template argument deduction, specialisation, dependent names | Advanced | Covered |
| 65 | Concepts & Constraints | Standard concepts, writing custom concepts, requires clauses, constrained overloads | Advanced | Covered |
| 66 | Ranges & Views | Range algorithms, lazy views, composing pipelines, projections | Advanced | Covered |
| 67 | Smart Pointers & Ownership | unique_ptr, shared_ptr and weak_ptr, ownership transfer, breaking reference cycles | Advanced | Covered |
| 68 | Move Semantics | Lvalues and rvalues, move constructors and assignment, the rule of five, std::move vs copy | Advanced | Covered |
| 69 | Operator Overloading | Arithmetic and comparison operators, the spaceship operator, stream operators, invariants | Advanced | Covered |
| 70 | Compile-time Programming | constexpr and consteval functions, static_assert, type traits, if constexpr | Advanced | Covered |
| 71 | Functional Tools | Lambdas and captures, std::invoke, std::bind_front, higher-order functions, memoisation | Advanced | Covered |
| 72 | Design Patterns | Strategy, factory, decorator and command patterns in modern C++ | Advanced | Covered |
| 73 | Concurrency: Threads & Mutexes | std::thread, mutex and lock_guard, condition variables, producer-consumer queues | Advanced | Covered |
| 74 | Concurrency: Futures & Thread Pools | std::async, promises and futures, exception propagation, a fixed-size thread pool | Advanced | Covered |
| 75 | Coroutines | co_yield generators, co_await basics, promise types, lazy sequences | Advanced | Covered |
| 76 | Atomics & Memory Order | std::atomic, compare-exchange, memory ordering, lock-free counters and flags | Advanced | Covered |
| 77 | Logging & Configuration | Log levels, formatters and sinks, INI-style configuration, environment overrides | Advanced | Planned |
| 78 | Unit Testing & Test Doubles | Test fixtures, table-driven tests, fakes, stubs and mocks, dependency injection | Advanced | Planned |
| 79 | Build Systems & Packaging | CMake targets and usage requirements, install and export rules, versioning, CPack | Advanced | Planned |
| 80 | Profiling & Performance | Measuring with std::chrono, benchmark harnesses, data layout and cache locality, algorithmic wins | Advanced | Planned |
| 81 | Regular Expressions | std::regex matching and searching, capture groups, replacement, redacting sensitive data | Advanced | Planned |
| 82 | Building a Storage Engine | Append-only logs, in-memory indexes, crash recovery, compaction and transactions | Advanced | Planned |

## Phase 5 · Capstone-Style Systems Projects (Days 83–100)

**By the end of this phase you can** deliver a multi-module, tested, logged, configured and packaged tool you can show an employer.

| Day | Topic | Key Learnings / Deliverables | Level | Status |
|----:|-------|------------------------------|-------|--------|
| 83 | Robust CLI Application | Subcommands, configuration files with overrides, verbosity flags, JSON output and exit codes | Capstone | Planned |
| 84 | Data Pipeline / ETL | Extract, transform and load stages, validation with quarantine, run summaries | Capstone | Planned |
| 85 | Concurrent File Processor | Work distribution over a thread pool, checksums, per-item error reporting | Capstone | Planned |
| 86 | Custom Logging & Monitoring Tool | Structured JSON logs, log rotation, metrics in Prometheus text format, alert thresholds | Capstone | Planned |
| 87 | Plugin-style Architecture | Plugin interfaces, self-registration, factories, versioned compatibility checks | Capstone | Planned |
| 88 | Automated Report Generator | Aggregation with exact integer money, templated HTML, CSV export, escaping | Capstone | Planned |
| 89 | Background Task Scheduler | Schedule specifications, an injectable clock, timeouts, no overlapping runs | Capstone | Planned |
| 90 | Memory-efficient Large File Processor | Streaming in fixed-size chunks, line reassembly, external merge sort | Capstone | Planned |
| 91 | Type-safe Configuration System | Typed settings, parsing and validation from environment variables, collecting every error, secret redaction | Capstone | Planned |
| 92 | Test Suite for a Multi-module Library | Separate model, repository, notifier and service modules, fakes and a fixed clock, business-rule tests | Capstone | Planned |
| 93 | Network Service Core | A line protocol, per-connection sessions, broadcasting, a transport-independent server core | Capstone | Planned |
| 94 | Data Validation & Cleaning Library | Composable validators, error paths, normalising messy input, custom exceptions | Capstone | Planned |
| 95 | Performance-critical Module | Naive vs spatial-index nearest-neighbour search, benchmarks, cross-checked results | Capstone | Planned |
| 96 | Packaging a Real Tool | Single-sourced versions, generated usage docs, changelogs, release pre-flight checks | Capstone | Planned |
| 97 | Automation Bot Suite | Combining scraping, APIs, scheduling and notifications with deduplication and retries | Capstone | Planned |
| 98 | Scientific Simulation | SIR epidemic model with RK4 integration, stochastic Monte Carlo, reproducible seeds | Capstone | Planned |
| 99 | Observability & Debugging Toolkit | Crash reports, nested timed spans, call tracing, watchpoints on values | Capstone | Planned |
| 100 | Portfolio Capstone: Production-ready C++ Tool | A multi-module tool with a CLI, configuration, storage, reports, logging, packaging and a full test suite | Capstone | Planned |
