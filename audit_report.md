# Audit Report: 100-Day C++ Systems Architecture Challenge

**Repository:** `OdeToTheWind/cpp-systems-architecture`
**Branch audited:** `claude/wizardly-cerf-dyrzoz` (head `8b76b05`, "list prepared")
**Audit date:** 2026-10-01
**Toolchain used for verification:** GCC 13.3, CMake + Ninja, `-std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion` (the flags in `CMakeLists.txt`), plus UBSan and `_GLIBCXX_ASSERTIONS` for targeted checks.

---

## 0. Scope, Method and Deviations From the Brief

The brief describes a layout that differs from the repository in four ways. Each was handled as follows:

| Brief says | Repository actually has | How the audit handled it |
|---|---|---|
| `syllabus.md` with columns `Day \| Topic \| Key Learnings/Deliverables \| Topic Level \| Status of covered` | **No `syllabus.md` exists** in any branch or in git history | The **"Daily Progress" tables in `README.md`** (lines 168–293) are the only syllabus. They have the columns `Day \| Topic \| Status \| Key Focus / Deliverables`. That table is the source of truth below. |
| A `Topic Level` column | Not present | Level is inferred from the README section headings: Days 1–24 = **Beginner**, 25–64 = **Intermediate**, 65–80 = **Advanced**, 81–100 = **Portfolio**. |
| `tests/test_day_XX.py` | `tests/test_day_XX.cpp` (plain `assert` + `main`) | The C++ test files were audited. |
| `docs/progress/day-XX-reflection.md` | `docs/progress/day_XX_<topic>.md` | The actual files were audited. |

**Method.** Every file under `src/`, `tests/` and `docs/progress/` was read in full (≈4.9k lines). All 29 day executables were built with the project's CMake. All 29 test files were compiled and run individually with the project's warning flags. Suspected runtime bugs were then confirmed by piping crafted input into the built `src/` executables. Every bug marked **(verified)** below was reproduced; the input used is shown.

---

## 1. Executive Summary

### 1.1 Headline numbers

| Metric | Value |
|---|---|
| Syllabus days | 100 |
| Days with any code in `src/` | **29** (Days 01–29) |
| Days 30–100 | 0 files (all correctly marked ⏳ Planned) |
| **Overall completion vs. 100-day syllabus** | **20 %** (Pass = 1, Partial = 0.5, Missing = 0 → 12 Pass + 16 Partial + 72 Missing) |
| Completion of days 1–29 (attempted) | **69 %** (12 Pass, 16 Partial, 1 Missing) |
| Completion of days README marks ✅ Completed (1–23) | **74 %** (11 Pass, 12 Partial) |
| `src/` build | ✅ 29/29 targets build with **zero warnings** on GCC 13 |
| Test compile | ✅ 29/29 compile (2 warnings) |
| **Test run** | ❌ **28 pass / 1 FAIL** (`test_day_16`: `Assertion 'get_grade(101) == "Invalid"' failed`, exit 134) |
| Tests wired into CMake / CTest / CI | ❌ **0 / 29** — tests are never built or run automatically |
| Tests that exercise real `src/` code | ❌ **1 / 29** (only Day 02 includes a `src/` header) |
| Tests with zero assertions | 3 (Days 25, 26, 29) |
| Tests that only assert on the standard library or tautologies | 4 more (Days 01, 17, 23, and the string checks in 14) |
| Days where reflection docs claim features absent from code | **18** (Days 01, 05–17, 20, 22, 24, 28; see §3) |
| Status mismatches README ↔ docs | **6** (Days 24–29: README "⏳ Planned", docs "**Status:** Completed", code present) |

### 1.2 Pass / fail test status

```
test_day_01 .. test_day_15   PASS
test_day_16                  FAIL  tests/test_day_16.cpp:32  get_grade(101) == "Invalid"
test_day_17 .. test_day_29   PASS
```

> ⚠️ **"PASS" overstates what is tested.** 28 of 29 suites define their own copy of the logic inside the test file and assert against that copy. They never link or include the code in `src/`. A passing test therefore says nothing about `src/`. Two examples: `tests/test_day_05.cpp:20-22` asserts the *buggy* discount formula, and the Day 16 failure is the only case where a copy kept the real bug.
>
> All tests use `assert`, so building them with `-DNDEBUG` (any Release config) turns every check into a no-op. **(Verified:** `test_day_16` "passes" when built with `-DNDEBUG`.)

### 1.3 Key systemic flaws (ranked)

1. **Tests are disconnected from the code.** Logic lives inside `main()` in `src/day_XX/main.cpp`, where it cannot be called. Tests re-implement it, and the copies have drifted from `src/` (Days 05, 11, 18, 20, 27, 28).
2. **The test suite is never executed.** `CMakeLists.txt` has no `enable_testing()` or `add_test()`. CI (`.github/workflows/cpp-ci.yml`) only builds `src/`. A failing test (Day 16) has gone unnoticed.
3. **Input handling is inconsistent across days.** Recurring defects:
   - unchecked `std::cin >>` leaves the stream in a failed state, so later reads cascade (Days 1, 4, 7, 9, 10, 11, 12, 13, 14, 16–28);
   - `std::cin.ignore()` discards exactly **one** character (Days 14, 17, 18, 24), which loses input (verified);
   - loops that ignore EOF **hang forever** (Days 10, 15, 17; verified).
4. **Undefined behaviour and crashes on user input**, despite the README's claim of a "zero UB focus" (README.md:165):
   - Day 10 **segfaults** when min > max;
   - Day 13 overflows a signed integer (confirmed by UBSan);
   - Day 10 passes an out-of-range probability to `std::bernoulli_distribution`.
5. **The reflection docs over-claim.** They describe features that were never written: dice roller, password generator, custom exception, re-throw, De Morgan demo, "remove" in Day 17, max-of-three, factorial, buggy functions to debug, destructors, and more. Treated as a syllabus record, the docs are unreliable.
6. **The two status sources disagree.** README marks Days 24–29 ⏳ Planned. The docs mark them Completed. The code for them is a placeholder or prints text only.
7. **Missing standard headers** (`<stdexcept>` in 4 `src` files; `<cmath>` and `<cctype>` in 2 tests). The code compiles only because GCC 13's libstdc++ pulls these in indirectly, so it is not portable.
8. **Build and CI hygiene problems:**
   - `CMakeLists.txt:52` prints `${CMAKE_ARGC}` and emits "Configured  day executables" with no count (verified);
   - `file(GLOB)` is used without `CONFIGURE_DEPENDS`;
   - the CI verification step can never fail (`|| echo`);
   - CI does not pass `-Werror`;
   - `procpp.sh` only lists `*.exe` files, so it reports nothing on Linux or macOS.

---

## 2. Gap Analysis Table

**Status legend:**
- **Pass:** every README deliverable is implemented in `src/` and gives correct results for valid input.
- **Partial:** at least one README deliverable is missing, or gives wrong results for valid input, or its test fails.
- **Missing:** nothing substantive is implemented.

The **Tests** column describes the matching file in `tests/`.

| Day | Level | Syllabus claim (README) | README status | Actual repo state | Tests | Status |
|---|---|---|---|---|---|---|
| 01 | Beginner | Fundamental types, initialization, `cin`/`cout`/`getline`, **mini calculator** | ✅ | Types and I/O shown. **No mini calculator.** No input validation. | Tests `std::string` only, not Day 1 code | **Partial** |
| 02 | Beginner | `length`, `substr`, `find`, **concatenation, formatting, input cleaning** | ✅ | length/substr/find and palindrome present. No concatenation, formatting or trimming in `src`. Header has an ODR bug. | ✅ Good: only suite that tests real `src` code | **Partial** |
| 03 | Beginner | Console I/O patterns, buffer cleaning, formatted output | ✅ | Implemented (age, name, gender, BMI, `iomanip`) | Re-implemented copy; one `-Wconversion` warning | **Pass** |
| 04 | Beginner | snake_case, intention-revealing names, anti-patterns | ✅ | Implemented. Height 0 gives BMI `inf`. | Tests its own names only | **Pass** |
| 05 | Beginner | Arithmetic, **compound assignment**, `<cmath>` | ✅ | **Discount is *added* to the price** (logic bug). Only `+=` and `*=` shown. | Test asserts the buggy result (137.5) | **Partial** |
| 06 | Beginner | Fundamental types, **modifiers**, sizes/ranges, fixed-width | ✅ | `char` limits print as raw bytes. Most modifiers missing (`long`, `unsigned short`, `long double`, `bool` …). | Platform checks only | **Partial** |
| 07 | Beginner | Implicit vs explicit, `static_cast`, **`dynamic_cast`**, narrowing | ✅ | No `dynamic_cast` (nor `const_`/`reinterpret_cast`). The "implicit" section uses explicit casts. Narrowing appears only in a comment. | Re-implemented; missing `<cmath>` | **Partial** |
| 08 | Beginner | Decision making, input validation, early returns | ✅ | Implemented well | Re-implemented copy; no boundary tests | **Pass** |
| 09 | Beginner | Combining conditions, **short-circuiting, De Morgan's laws** | ✅ | `&&`/`||`/`!` shown. Short-circuit and De Morgan are never demonstrated in code (only printed as text). | Re-implemented copy | **Partial** |
| 10 | Beginner | `std::mt19937`, uniform distributions, modern RNG | ✅ | Implemented. **Segfault when min > max**, NaN with 0 flips, hangs on EOF. | Non-deterministic seed; tests the std lib, not `src` | **Pass** (with high-severity flaws) |
| 11 | Beginner | Exceptions, `std::runtime_error`, input validation | ✅ | Implemented. Throw/catch within one scope (exceptions used as control flow). No custom exception, even though a comment claims one. | Copy has drifted from `src` | **Pass** (doc over-claims) |
| 12 | Beginner | Declaration, parameters, return values, overloading | ✅ | Implemented. Missing `<stdexcept>`. No separate prototypes. | Re-implemented; double overload untested | **Pass** |
| 13 | Beginner | Traditional and range-based for, nested loops | ✅ | Implemented. Signed overflow in the multiplication table (UB). | Tests `factorial`, which is not in `src` | **Pass** |
| 14 | Beginner | Scoping, consistent style, readability | ✅ | Implemented. `cin.ignore()` bug loses the name. Doc claims Allman style; the code uses something else. | Tautological string asserts | **Pass** |
| 15 | Beginner | Pre-test vs post-test loops, menu systems | ✅ | Implemented. Menu asks for operands before validating the choice. Hangs on EOF. | Re-implemented copy | **Pass** |
| 16 | Beginner | Translating logic flow into clean C++ | ✅ | Score > 100 is graded "B". No flowchart artefacts anywhere. | ❌ **FAILS** (`test_day_16.cpp:32`) | **Partial** |
| 17 | Beginner | `std::vector` and `std::map` | ✅ | Implemented, but **first shopping item loses its first character** (verified). No remove/erase, though docs claim it. | Tests only the std lib | **Partial** |
| 18 | Beginner | Default args, overloading, **Named Parameter Idiom** | ✅ | Defaults and overloads shown. "NPI" is a plain parameter struct, not the chained-setter idiom. `width == 0.0` used as a sentinel. | Tests `describe_person`, which does not exist in `src` | **Partial** |
| 19 | Beginner | Raw pointers, references, pass-by-value vs reference | ✅ | Implemented | Pointer functions untested; unused-variable warning | **Pass** |
| 20 | Beginner | **Function pointers**, `std::function`, **callbacks** | ✅ | `std::function` and lambdas shown. **No function pointers. No callback** (no function takes a function). Unknown op silently becomes `+`. | Copy lacks subtract/divide | **Partial** |
| 21 | Beginner | Function design, side effects vs pure functions | ✅ | Implemented. Missing `<stdexcept>`. Good/bad versions support different operators. | Re-implemented copy; `/` success untested | **Pass** |
| 22 | Beginner | Comments vs docstrings, **generating documentation** | ✅ | Doxygen comments present. **No Doxyfile and no doc target.** Missing `<stdexcept>`. | Re-implemented copy | **Partial** |
| 23 | Beginner | Lifetime, **shadowing**, namespace usage | ✅ | Global/local/namespace counters shown. **No shadowing and no lifetime demo.** | Tautology (`local += 5`) | **Partial** |
| 24 | Beginner | GDB basics, debugging strategies | ⏳ (docs: Completed) | Text only. The "buggy" function is the correct version. No `std::cerr`, no GDB walkthrough. | Copy of `sum_to_n` | **Partial** |
| 25 | Intermediate | IDE/compiler setup, CMake best practices | ⏳ (docs: Completed) | `main` prints a bullet list. CMake best practices not applied (see §3 infra). | 0 asserts | **Partial** |
| 26 | Intermediate | Advanced editor features, debugging in IDE | ⏳ (docs: Completed) | `main` prints a bullet list. Refers to `calculate_bmi`, which is not in the file. No `.clang-format`. | 0 asserts | **Partial** |
| 27 | Intermediate | OOP principles in C++ | ⏳ (docs: Completed) | Encapsulation only. **No abstraction, inheritance or polymorphism.** | Tests a separate `TestPlayer`, not `Player` | **Partial** |
| 28 | Intermediate | Class definition, encapsulation | ⏳ (docs: Completed) | `BankAccount` implemented. `withdraw` never exercised. No destructor, though docs claim one. | Tests a separate `TestAccount` | **Pass** |
| 29 | Intermediate | Header-only vs linking, vcpkg/Conan | ⏳ (docs: Completed) | Placeholder that prints future plans. **No library integrated.** | 0 asserts | **Missing** |
| 30–64 | Intermediate | (see README) | ⏳ | No `src/`, `tests/` or docs | none | **Missing** (status consistent) |
| 65–80 | Advanced | (see README) | ⏳ | No `src/`, `tests/` or docs | none | **Missing** (status consistent) |
| 81–100 | Portfolio | (see README) | ⏳ | No `src/`, `tests/` or docs | none | **Missing** (status consistent) |

---

## 3. Detailed Flaws & Findings (Grouped by Day)

Severity tags: 🔴 High (crash, UB, wrong result, failing test) · 🟠 Medium (missing deliverable, data loss, doc/status mismatch) · 🟡 Low (style, robustness, hygiene).

### Repository-wide infrastructure

**Code flaws**
- 🔴 `CMakeLists.txt` has no `enable_testing()` / `add_test()`. Nothing in `tests/` is compiled by the build. README.md:324 still lists `ctest -V` as "planned".
- 🔴 `.github/workflows/cpp-ci.yml:40-46`: CI runs `./procpp.sh` only. There is no test step. The "Check built executables" step ends in `|| echo …`, so it can never fail. CI runs only on Windows and only for `main`.
- 🟠 `CMakeLists.txt:52`: `${CMAKE_ARGC}` is the CMake command-line argument count in script mode, not a target count. The output is `Configured  day executables` **(verified)**. Use a counter variable incremented in the loop.
- 🟡 `CMakeLists.txt:18,28`: `file(GLOB …)` without `CONFIGURE_DEPENDS`. A new `day_XX` folder is ignored until CMake is re-run by hand.
- 🟡 `CMakeLists.txt:11-15`: warnings are enabled, but neither the build nor CI uses `-Werror`, so new warnings (e.g. in tests) would pass silently. No sanitizer preset exists.
- 🟡 `procpp.sh:19-29` globs `build/day_*.exe` only. On Linux or macOS it always prints "(no day executables found yet)".

**Deliverable / sync gaps**
- 🟠 No `syllabus.md`. The README tables are the only syllabus, and the docs keep their own status lines.
- 🟠 README tree does not match the files on disk:
  - README.md:33 lists `day_01_memory.md`; the actual file is `day_01_variables.md`;
  - README.md:52 and :103 list `day_20_returning_functions`; the actual name is `day_20_returning_function`;
  - `docs/progress/day_20_returning_function.md` itself points to `src/day_20_returning_functions/main.cpp`, which does not exist.
- 🟡 README.md:335-end contains a leftover HTML comment of chat-assistant boilerplate ("You can copy-paste this directly into your README.md … Good luck with Day 2").
- 🟡 The "Next Day Preview" sections contradict the syllabus. Day 10 previews "Loops begin", Day 11 previews "Loops introduction", Day 12 previews "Arrays and `std::vector`". The actual Days 11, 12 and 13 are error handling, functions and for-loops.

**Test coverage (systemic)**
- 🔴 28/29 tests re-implement logic instead of calling `src/`. The root cause is that each day keeps all logic inside `main()`.
- 🟠 `assert` with no test framework: under `NDEBUG` the suite silently checks nothing **(verified)**. The README plans Catch2/GoogleTest; neither is integrated.

---

### Day 01 — Variables, Types & Basic I/O (`src/day_01_memory/main.cpp`)
- **Code flaws**
  - 🟡 `:29`, `:32` — `std::cin >> userAge` and `>> userSalary` are unchecked.
  - 🟡 `:11`, `:21-23` — `isStudent`, `userName`, `userAge` use camelCase. Day 04 later sets snake_case as the project convention.
  - 🟡 The folder name `day_01_memory` does not match the topic or the doc name (`day_01_variables.md`).
- **Deliverable gaps**
  - 🟠 README's **"mini calculator"** is not implemented.
  - The doc says "Forgetting to initialize variables" was fixed, but `userAge` and `userSalary` are still declared uninitialized.
- **Test coverage**
  - 🟠 `tests/test_day_01.cpp:5-7` asserts `std::string` length and concatenation. That is Day 02 material and is unrelated to anything in Day 01's `main.cpp`.

### Day 02 — String Manipulation (`src/day_02_strings/`)
- **Code flaws**
  - 🔴 `string_utils.hpp:8` — `isPalindrome` is a **non-`inline` function defined in a header**. Including it from two translation units fails to link with `multiple definition of isPalindrome` **(verified)**. Fix: mark it `inline`, or move the body to a `.cpp` file.
  - 🟠 `string_utils.hpp:13` — `std::isalpha` drops digits, so `isPalindrome("1a2")` returns **true** **(verified)**. Use `std::isalnum` if digits should count.
- **Deliverable gaps**
  - 🟠 README lists concatenation, formatting and input cleaning. None of these appear in `src`; concatenation exists only in `test_day_01`.
- **Test coverage**
  - ✅ The best suite in the repo: it includes the real header and covers edge cases.
  - 🟡 `:17` — asserting that `"!!! 123 !!!"` is a palindrome locks in the digit-dropping behaviour. There is no mixed digit/letter case.

### Day 03 — Input & Print (`src/day_03_prints/main.cpp`)
- **Code flaws**
  - 🟡 `:58-62` — height and weight are not checked with `cin.fail()`. A failed read becomes 0 and is reported as "must be positive", which is misleading.
  - 🟡 `:95-97` — the gender-specific BMI note is a hard-coded health claim. Keep it clearly framed as illustrative.
- **Test coverage**
  - 🟡 `tests/test_day_03.cpp:8-23` re-implements the logic.
  - 🟡 `:18` raises a `-Wconversion` warning and uses `std::tolower` without `<cctype>`.
  - 🟡 The copy maps unknown gender to `"Invalid"`, but `src` falls back to `"Other"` (drift).
  - 🟡 BMI category boundaries (18.5, 25, 30) are untested.

### Day 04 — Variable Naming (`src/day_04_variable_names/main.cpp`)
- **Code flaws**
  - 🟡 `:8-12` — mutable globals are presented as "good naming examples". That contradicts Day 23's "avoid globals".
  - 🟡 `:50-59` — no validation. Height `0` gives `BMI: inf` and "Above limit" **(verified)**.
- **Test coverage**
  - 🟡 `tests/test_day_04.cpp` checks names it declares itself. This is acceptable for a naming lesson, but it does not touch `src`.

### Day 05 — Mathematical Operations (`src/day_05_maths_operations/main.cpp`)
- **Code flaws**
  - 🔴 `:54` — `price += discount_amount;` **adds** the discount; it should be `-=`. The comment on the line ("wait — usually discount first") shows the author noticed. With price 100, tax 10 % and discount 25, the program prints **137.50** where the correct answer is **82.50** **(verified)**.
  - 🟡 `:84` — `std::sqrt` of a negative input prints `nan` with no guard.
  - 🟡 `:12-96` — the same validated-read loop is copy-pasted 6 times. Extract a helper such as `read_number<T>(prompt)`.
  - 🟡 `:21`, `:24` — `a * b` can overflow, and `INT_MIN / -1` is UB.
- **Deliverable gaps**
  - 🟠 Only `+=` and `*=` appear. The doc claims `-=`, `/=` and `%=`. Decrement (`--`) and `std::hypot` appear only in the test or the doc.
- **Test coverage**
  - 🔴 `tests/test_day_05.cpp:20-22` asserts `100 + 25` then `× 1.1 == 137.5`, which **enshrines the bug**.

### Day 06 — Data Types (`src/day_06_data_types/main.cpp`)
- **Code flaws**
  - 🟠 `:21-29` — `numeric_limits<char>::min()/max()` and `<unsigned char>::max()` are streamed as characters. The output is raw bytes `\x80 → \x7f` and `0 → \xff` instead of `-128 → 127` and `0 → 255` **(verified)**. Wrap them in `static_cast<int>` or apply unary `+`.
  - 🟡 `:58-60`, `:63`, `:67` — unqualified `int32_t`, `uint64_t` and `size_t`. `<cstdint>` only guarantees the `std::` versions.
  - 🟡 `:8` — the alias `byte` is easy to confuse with C++17 `std::byte`.
- **Deliverable gaps**
  - 🟠 README promises "modifiers". The table omits `signed char`, `unsigned short`/`int`/`long long`, `long`, `long double` and `bool`. The doc additionally lists `wchar_t` and `char8/16/32_t`.
- **Test coverage**
  - 🟡 `tests/test_day_06.cpp:12` — `sizeof(double) >= 4` is a very weak check. The suite tests the platform, not `src`.

### Day 07 — Type Conversion & Casting (`src/day_07_convert_types_casting/main.cpp`)
- **Code flaws**
  - 🟠 `:24-33` — the "Implicit conversions" section uses `static_cast` in every example, so no implicit conversion is actually shown.
  - 🟡 `:41` and `:47` duplicate each other.
  - 🟡 `:10-20` — inputs are unvalidated.
  - 🟡 `:38` — user input is converted to unsigned with no range check, even though `:57-63` shows the "safe way".
- **Deliverable gaps**
  - 🟠 README claims `dynamic_cast`. There is no `dynamic_cast`, `const_cast` or `reinterpret_cast` example, and no polymorphic type to cast.
  - 🟠 Narrowing exists only as a comment (`:45`). Add a brace-init example such as `int{d}`, which fails to compile.
- **Test coverage**
  - 🟡 `tests/test_day_07.cpp:8` calls `std::abs(double)` without `<cmath>`, which is not portable. It does not test `src`.

### Day 08 — Conditionals (`src/day_08_if_else_conditionals/main.cpp`)
- **Code flaws:** none significant. The input validation and early returns are a good model.
- **Deliverable gaps**
  - 🟠 The doc claims a "login attempt checker" in `src` and "temperature category functions" in the test. Neither exists.
- **Test coverage**
  - 🟡 Re-implemented copy.
  - 🟡 No boundary tests at 89.99/90, 12/13, 19/20 or 64/65. Temperature advice is untested.

### Day 09 — Logical Operators (`src/day_09_logical_operators/main.cpp`)
- **Code flaws**
  - 🟠 `:16-22`, `:44-50` — `std::cin >> bool` accepts only `0`/`1`. Typing `yes` puts the stream into a failed state, and **every later prompt is silently skipped**: the license, insurance, purchase, member and weekend answers all become false or 0 **(verified)**.
  - 🟡 `:41-42` — the purchase amount is unvalidated.
- **Deliverable gaps**
  - 🟠 README promises short-circuiting and De Morgan's laws. Neither is demonstrated in code; they appear only in the closing text at `:85-90`.
  - 🟠 The doc claims a "safe division short-circuit" example and a "range check". Neither exists.
- **Test coverage**
  - 🟡 Re-implemented copy.
  - 🟡 Password boundary lengths (7 and 8) are untested.
  - 🟡 Short-circuit side effects are not tested.

### Day 10 — Randomisation (`src/day_10_randomisation/main.cpp`)
- **Code flaws**
  - 🔴 `:22` — `uniform_int_distribution(min_val, max_val)` with `min > max` violates its precondition. The release build **segfaults (exit 139)**, and `_GLIBCXX_ASSERTIONS` reports `Assertion '_M_a <= _M_b' failed` **(verified with input `10 1 5`)**.
  - 🔴 `:49` — `bernoulli_distribution(p)` requires `0 ≤ p ≤ 1`. With `p = 2` the run reports "100 % heads" with no error, which is UB **(verified)**.
  - 🔴 `:62` — with 0 flips, `heads / flips` prints `-nan% heads` **(verified)**.
  - 🟠 `:36` — `uniform_real_distribution` needs `min < max`. This is unchecked.
  - 🟠 `:74-92` — on EOF, `cin.clear()` plus `continue` loops forever **(verified: hangs until timeout)**.
- **Deliverable gaps**
  - 🟠 The doc claims a "Dice roller" and a "random password generator". Neither exists.
- **Test coverage**
  - 🟡 `tests/test_day_10.cpp:8-9` uses `random_device` seeding, so runs are not reproducible. Use a fixed seed in tests.
  - 🟡 `:22` checks `val <= 1.0`, but the distribution is half-open `[0, 1)`, so assert `< 1.0`.
  - 🟡 The suite tests `<random>`, not `src`.

### Day 11 — Error Handling (`src/day_11_error_handling/main.cpp`)
- **Code flaws**
  - 🟠 `:12` — `std::cin >> num1 >> num2` is unchecked. Input `10 abc` sets `num2 = 0` and the program reports "Division by zero is not allowed!", which is wrong **(verified)**.
  - 🟡 `:14-23`, `:61-73` — the code throws and catches in the same scope, which uses exceptions as local control flow. The Day 11 doc itself says "Don't use exceptions for normal control flow". Move the throw into a validator function.
  - 🟡 No `catch (const std::exception&)` fallback.
- **Deliverable gaps**
  - 🟠 The `:56` comment says "Custom exception", but the code uses `std::runtime_error`; no custom exception class exists.
  - 🟠 The doc claims "array access with bounds checking" and "added `throw;` example". Neither exists.
- **Test coverage**
  - 🟡 `tests/test_day_11.cpp:42-48` — `validate_age(25)` and `validate_age(-5)` share one `try`. If the valid call wrongly threw, the test would still pass.
  - 🟡 Age 151 is untested.
  - 🟡 The copy of `is_valid_password` has no space rule, so it has drifted from `src`.

### Day 12 — Functions (`src/day_12_functions/main.cpp`)
- **Code flaws**
  - 🟠 `:7`, `:19` — uses `std::invalid_argument` without `#include <stdexcept>`. It compiles only through libstdc++'s indirect includes.
  - 🟡 `:54`, `:66`, `:95-97` — inputs are unvalidated.
- **Deliverable gaps**
  - 🟡 README says "Declaration", but there are no separate prototypes or header.
  - 🟠 The doc claims factorial and rectangle area/perimeter helpers. They do not exist.
- **Test coverage**
  - 🟡 The `describe_number(double)` overload is untested.
  - 🟡 `calculate_bmi`'s throw path is untested.
  - 🟡 Re-implemented copy.

### Day 13 — For Loops (`src/day_13_for_loops/main.cpp`)
- **Code flaws**
  - 🔴 `:36` — `num * i` overflows a signed `int`. UBSan reports `signed integer overflow: 21475 * 100000` **(verified)**. Use `long long` and bound `terms`.
  - 🟡 `:58` — the prompt says "random numbers", but the values are deterministic (`i*10+5`).
  - 🟡 `:11`, `:27`, `:31` — `cin` reads are unchecked.
- **Deliverable gaps**
  - 🟡 The doc claims factorial plus `break`/`continue` demos. Factorial exists only in the test; `break` and `continue` are not used.
- **Test coverage**
  - 🟡 Tests `factorial`, which is not in `src`. No overflow boundary test.

### Day 14 — Code Blocks & Indentation (`src/day_14_code_block_indentation/main.cpp`)
- **Code flaws**
  - 🟠 `:58` — `std::cin.ignore()` discards one character. Input `80 ` with a trailing space leaves `'\n'` in the buffer, so `getline` reads an empty name and prints "Name cannot be empty" **(verified)**.
  - 🟡 `:10`, `:33` — unchecked reads. Non-numeric age becomes 0 and is classed as "Child".
- **Deliverable gaps**
  - 🟡 The doc states "this project uses Allman style". The code uses K&R opening braces with `}` and `else` on separate lines, which is neither style. There is also trailing whitespace on lines 15, 18, 21, 24 and others.
  - 🟡 The doc claims "correct vs incorrect" and "dangling else" examples. They are absent.
- **Test coverage**
  - 🟡 `tests/test_day_14.cpp:35-39` asserts properties of string literals, which is tautological.
  - 🟡 The grade thresholds (75/60) differ from Days 08 and 16 (80/70/60). That is fine, but undocumented.

### Day 15 — While / Do-While (`src/day_15_while_loops/main.cpp`)
- **Code flaws**
  - 🟠 `:61` — `std::cin >> choice` is unchecked. A non-numeric choice or EOF produces an **infinite loop** **(verified: hang)**.
  - 🟠 `:69-81` — operands are requested **before** the menu choice is validated. Choice 9 prompts "Enter two numbers" and only then prints "Invalid choice" **(verified)**.
  - 🟡 `:24-29` — the guess loop hangs on EOF for the same reason as Day 10.
- **Deliverable gaps**
  - 🟡 The doc claims an "input validation loop" and a "sum calculator until user chooses to stop". Neither exists.
- **Test coverage**
  - 🟡 Re-implemented helpers. The menu and guessing logic are untested.

### Day 16 — Flowchart Programming (`src/day_16_flowchart_programming/main.cpp`)
- **Code flaws**
  - 🔴 `:36-48` — only the first branch has an upper bound (`score <= 100`), so a score of 150 falls through and prints **"B - Very Good"** **(verified)**. Validate the range first.
  - 🔴 **The test fails**: `tests/test_day_16.cpp:32` reports `Assertion 'get_grade(101) == "Invalid"' failed` (exit 134). The copy kept the same bug.
  - 🟡 `:9`, `:14` — the variable is named `has_license`, but the prompt asks about a *learner's permit*.
  - 🟡 `:68` — `int balance = 5000.0;` initializes an `int` from a `double` literal.
  - 🟡 No `cin` validation anywhere.
- **Deliverable gaps**
  - 🟠 A flowchart-programming day ships **no flowcharts**: no diagram, Mermaid or image in `docs/`.
  - 🟠 The doc claims "Finding maximum of three numbers". It does not exist.
- **Test coverage**
  - 🔴 Failing (see above).
  - 🟡 Eligibility logic is untested.
  - 🟡 Grade boundaries 90/80/70/60 are only partly covered.

### Day 17 — Vectors & Maps (`src/day_17_maps_vectors/main.cpp`)
- **Code flaws**
  - 🔴 `:71` — the extra `std::cin.ignore()` after the menu loop **eats the first character of the first shopping item**. `milk` is stored as `ilk` **(verified)**. `:25` already consumed the newline, so remove `:71`.
  - 🟠 `:24` (menu) and `:73-85` (`while(true)` + `getline`) — both **loop forever on EOF**.
  - 🟡 `:31` — the score is not validated against 0–100, despite the prompt.
  - 🟡 `:44` — use `const auto& [student, s]` (C++17 structured bindings).
- **Deliverable gaps**
  - 🟠 The doc claims "add, view, search, **and remove**". There is no `erase` for the map or the vector.
- **Test coverage**
  - 🟠 `tests/test_day_17.cpp` only exercises `std::vector` and `std::map` themselves. Nothing from `src` is tested.

### Day 18 — Positional & Keyword Arguments (`src/day_18_positional_keyword_arguments/main.cpp`)
- **Code flaws**
  - 🟠 `:10-11` — `width == 0.0` is used as an "omitted" sentinel. An explicit `calculate_rectangle_perimeter(10, 0)` silently becomes a square. Use an overload or `std::optional<double>`.
  - 🟡 `:62-66` — "Enter name and age" reads the name with `getline`, then the age on the next line. The prompt is ambiguous. `:63` and `:87` use a single-character `ignore()`.
  - 🟡 `:91-92` — height is labelled "optional", but `cin >>` blocks until a number is entered.
- **Deliverable gaps**
  - 🟠 README says "Named Parameter Idiom". The struct at `:26-31` is a *parameter object*. The NPI proper uses chained setters that return `*this` (e.g. `PersonConfig().name("A").age(3)`). C++20 designated initializers would be the modern alternative, but the project is pinned to C++17.
- **Test coverage**
  - 🟠 `tests/test_day_18.cpp:16` tests `describe_person`, which does not exist in `src`.
  - 🟡 The `print_person_info` overloads and the NPI are untested. They print instead of returning, which ironically is Day 21's lesson.

### Day 19 — Pointers & References (`src/day_19_pointer_references/main.cpp`)
- **Code flaws**
  - 🟡 `:21` — a utility function prints its error, mixing logic with I/O (see Day 21).
  - 🟡 `swap_by_value` and `swap_by_reference` duplicate Day 12 verbatim.
- **Deliverable gaps** (none against README; depth is light for a "systems" repo)
  - 🟡 Missing: `const int*` vs `int* const`, pointer arithmetic, dangling pointers or references, heap allocation and why to prefer smart pointers.
- **Test coverage**
  - 🟠 `tests/test_day_19.cpp:26-27` — the unused `int* ptr = nullptr;` triggers `-Wunused-variable`. The comment says null pointers "can't easily" be tested; `swap_by_pointer(nullptr, &x)` followed by asserting `x` is unchanged is trivial.
  - 🟡 `swap_by_pointer` and `increment_by_pointer` are untested.

### Day 20 — Returning Functions (`src/day_20_returning_function/main.cpp`)
- **Code flaws**
  - 🟠 `:18` — `std::runtime_error` is used without `<stdexcept>`.
  - 🟠 `:22-25` — an unknown operator **silently falls back to addition**: `pow 2 3` gives `Result of pow: 5` **(verified)**. The pricing function at `:39-41` behaves the same way. Return an empty `std::function` or `std::optional`, or throw.
- **Deliverable gaps**
  - 🟠 README promises **function pointers**; none are used.
  - 🟠 README promises **callbacks**; no function accepts a function parameter. The doc's "dynamic callback system" does not exist.
- **Test coverage**
  - 🟡 The copy of `get_operation` has only add and multiply.
  - 🟡 Subtract, divide, divide-by-zero and the pricing strategies are untested.

### Day 21 — Return vs Print (`src/day_21_return_vs_print/main.cpp`)
- **Code flaws**
  - 🟠 `:21` — `std::invalid_argument` is used without `<stdexcept>`.
  - 🟡 `:5-13` vs `:16-22` — the "bad" calculator supports only `+` and `-`, while the "good" one supports `+ - * /`. The comparison is not like-for-like.
  - 🟡 `:20-21` — "invalid operator" and "division by zero" collapse into one exception type and message.
- **Test coverage**
  - 🟡 `tests/test_day_21.cpp:19-22` — `catch (...) {}` swallows everything.
  - 🟡 A successful `/` is never tested.

### Day 22 — Documentation (`src/day_22_docs_strings_comments/main.cpp`)
- **Code flaws**
  - 🟠 `:13` — `std::invalid_argument` is used without `<stdexcept>`.
  - 🟡 `:28-36` — a negative age is silently treated as "skip".
- **Deliverable gaps**
  - 🟠 README promises "generating documentation", but there is **no `Doxyfile`** and no `doxygen` target in CMake.
  - 🟠 The doc claims a "comparison between poor and good documentation". It is absent.
- **Test coverage**
  - 🟡 `calculate_circle_area`, including its throw path, is untested.

### Day 23 — Scope (`src/day_23_scope_local_global_variables/main.cpp`)
- **Code flaws**
  - 🟡 `:5` — `global_counter` has external linkage.
  - 🟡 `:9` — `static` inside a named namespace; the idiomatic form is an unnamed namespace.
  - 🟡 `:33` — the output says "shared across functions", but only `main` exists.
- **Deliverable gaps**
  - 🟠 README promises **shadowing**; no example.
  - 🟠 README promises **variable lifetime**; there is no `static` local, no block-scope end of life, and no object destruction.
- **Test coverage**
  - 🟠 `tests/test_day_23.cpp:7-9` asserts `10 + 5 == 15`, which is tautological.

### Day 24 — Debugging Techniques (`src/day_24_debugging_technique/main.cpp`)
- **Status sync:** 🟠 README.md:195 says ⏳ Planned; the doc says **Completed**.
- **Code flaws**
  - 🟠 `:9` — the comment says the function "has off-by-one bug", but the code is the correct version (`:15`), so there is nothing to debug.
  - 🟠 `:23` — the single-character `ignore()` means input `5 ` with a trailing space makes `getline` read an empty string. The bug report is silently dropped **(verified)**.
- **Deliverable gaps**
  - 🟠 README lists "GDB basics, debugging strategies". There is no GDB session or notes, no `std::cerr` use (despite `:32`), no `assert`, and no `-g`/sanitizer build preset.
  - 🟠 The doc claims "several buggy functions". There is one, and it is correct.
- **Test coverage**
  - 🟡 Third copy of the same sum-to-N loop (also in the Day 13 and Day 15 tests).

### Day 25 — Local Dev Environment (`src/day_25_dev_env_setup_local/main.cpp`)
- **Status sync:** 🟠 README ⏳ Planned vs doc Completed.
- **Deliverable gaps**
  - 🟠 `main` prints a static bullet list.
  - 🟠 README promises "CMake best practices", but none are applied: no `enable_testing`, no `CMakePresets.json`, no `CONFIGURE_DEPENDS`, no per-target warnings, and the `CMAKE_ARGC` bug is still there.
  - 🟡 `:10` prints "Testing: Manual + basic unit tests", even though tests are never built.
- **Test coverage**
  - 🟠 `tests/test_day_25.cpp` contains **zero assertions**.

### Day 26 — IDE Tips (`src/day_26_ide_tips_tricks/main.cpp`)
- **Status sync:** 🟠 README ⏳ Planned vs doc Completed.
- **Deliverable gaps**
  - 🟠 `main` prints a static bullet list.
  - 🟡 `:10` tells the reader to click `calculate_bmi`, which does not exist in this file.
  - 🟠 The doc recommends clang-format, but there is no `.clang-format` or `.clang-tidy`. Also, `.vscode/` is git-ignored, so no IDE configuration ships.
- **Test coverage**
  - 🟠 Zero assertions.

### Day 27 — OOP Basics (`src/day_27_oop_basics/main.cpp`)
- **Status sync:** 🟠 README ⏳ Planned vs doc Completed.
- **Code flaws**
  - 🟡 `:24-28` — `heal()` has no upper bound, so health can grow without limit. `heal()` is never called.
  - 🟡 `:12` — a single-argument converting constructor should be `explicit`.
  - 🟡 There is no `health()` getter, so the class cannot be tested.
- **Deliverable gaps**
  - 🟠 README says "OOP principles" and the doc's goals list Abstraction, Inheritance and Polymorphism. Only encapsulation is implemented.
- **Test coverage**
  - 🟠 `tests/test_day_27.cpp:5-14` tests a separate `TestPlayer` class, not `Player`. `heal` is untested.

### Day 28 — Classes (`src/day_28_classes/main.cpp`)
- **Status sync:** 🟠 README ⏳ Planned vs doc Completed.
- **Code flaws**
  - 🟡 `:7` — `double` is used for money; use integer minor units.
  - 🟡 `:10` — the constructor is not `explicit` and accepts a negative initial balance.
  - 🟡 `:13-23` — invalid deposits and withdrawals are silently ignored. Depositing `-50` leaves the balance unchanged with no message **(verified)**.
  - 🟡 `withdraw` is never called from `main`.
  - 🟡 `:26` prints `$`, while every other day uses Rupees.
- **Deliverable gaps**
  - 🟡 The doc goals mention destructors; there are none.
- **Test coverage**
  - 🟠 Tests a separate `TestAccount`, not `BankAccount`. `withdraw` is untested.

### Day 29 — External Libraries (`src/day_29_external_libraries/main.cpp`)
- **Status sync:** 🟠 README ⏳ Planned vs doc Completed. The doc is accurate in saying "we currently use pure standard library", which contradicts its own "Completed" status.
- **Deliverable gaps**
  - 🔴 No external library is integrated: no `FetchContent`, `find_package`, `vcpkg.json` or `conanfile`.
  - 🔴 There is no header-only vs. compiled-library example.
- **Test coverage**
  - 🟠 Zero assertions.

### Days 30–100
- Marked ⏳ Planned in the README, with no `src/`, `tests/` or `docs/` content. **The status is consistent with the repo state.**
- Several of these days depend on infrastructure that does not exist yet:
  - Days 37/47/86/93: SFML, Qt, SDL2;
  - Days 51/53/56–61: nlohmann/json, libcurl, Crow;
  - Days 44/71/77–80: Eigen, Armadillo, mlpack;
  - Days 66/67/72: SQLite.
- These depend on Day 29 (dependency management) being properly delivered first.

---

## 4. Actionable Remediation List (Prioritized)

### P0: Correctness and broken tests (do first)
1. **Fix the Day 16 grading bug and its failing test.** Validate `0 ≤ score ≤ 100` before the grade chain (`src/day_16_…/main.cpp:36`). Update the copy in `tests/test_day_16.cpp:7-14`, or better, test the real function (P1 #6).
2. **Fix the Day 05 discount bug** by changing `+=` to `-=` at `src/day_05_…/main.cpp:54`. Correct the assertion in `tests/test_day_05.cpp:20-22` to 82.5.
3. **Fix the Day 17 lost-character bug** by deleting the extra `std::cin.ignore()` at `src/day_17_…/main.cpp:71`.
4. **Fix the Day 10 crashes and UB:**
   - swap or reject the bounds when `min_val > max_val`, and require `min_d < max_d`;
   - clamp or reject probability outside [0, 1];
   - guard `flips == 0`.
5. **Fix the Day 13 overflow** by computing `static_cast<long long>(num) * i`, or by bounding `terms` and `num`.

### P1: Make the tests real and automatic
6. **Extract logic out of `main()`.** For each day, move the pure functions into `src/day_XX/<topic>.hpp` (plus a `.cpp`, or `inline`) and keep `main.cpp` as a thin I/O shell. Have each `tests/test_day_XX.cpp` `#include` that header instead of re-implementing it. This removes the drift found in Days 05, 11, 18, 20, 27 and 28.
7. **Wire the tests into CMake:**
   ```cmake
   enable_testing()
   file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/test_day_*.cpp")
   foreach(T ${TEST_SOURCES})
       get_filename_component(N ${T} NAME_WE)
       add_executable(${N} ${T})
       target_include_directories(${N} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)
       target_compile_options(${N} PRIVATE -UNDEBUG)   # keep assert active
       add_test(NAME ${N} COMMAND ${N})
   endforeach()
   ```
   Better still, adopt Catch2 or GoogleTest through `FetchContent`, which also delivers Day 29.
8. **Update CI** (`cpp-ci.yml`):
   - add a `ctest --output-on-failure` step;
   - remove `|| echo` from the verification step;
   - add `-Werror`;
   - add an Ubuntu job with `-fsanitize=address,undefined`;
   - run on PRs to any branch.
9. **Replace the zero-assertion and tautological tests** (Days 01, 17, 23, 25, 26, 29) with tests of real deliverables. For Days 25 and 26, a CMake configure check or a `clang-format --dry-run` CI step is a reasonable "test".

### P2: Input-handling hygiene (cross-cutting)
10. Add one shared helper, `include/io_utils.hpp`, with:
    - `template<class T> std::optional<T> read_value(std::istream&, std::string_view prompt)`, which handles fail, clear and ignore-to-newline, and **returns `nullopt` on EOF**;
    - `read_line(...)`.

    Use them everywhere. This fixes the EOF hangs (Days 10, 15, 17), the cascading-failure bugs (Days 9, 11), and the single-character `cin.ignore()` bugs (Days 14, 17, 18, 24, replaced by `ignore(numeric_limits<streamsize>::max(), '\n')` or `std::ws`).
11. Day 09: read yes/no as a string or char, or use `std::boolalpha` and document it.
12. Day 15: validate the menu choice before prompting for operands.

### P3: Close the deliverable gaps against the README syllabus
| Day | Add |
|---|---|
| 01 | Mini calculator (two numbers + operator) |
| 02 | Concatenation (`+`, `append`, `std::ostringstream`), a `trim()` input-cleaning helper; make `isPalindrome` `inline` and decide on digits (`isalnum`) |
| 05 | `-=`, `/=`, `%=`, `--`; guard `sqrt` of negatives |
| 06 | Full modifier table (`signed/unsigned short/int/long/long long`, `long double`, `bool`); cast `char` limits to `int` |
| 07 | `dynamic_cast` with a small polymorphic hierarchy; `const_cast`/`reinterpret_cast` with warnings; a real implicit-conversion example; brace-init narrowing |
| 09 | A short-circuit demo with a side-effect or null-pointer guard; a De Morgan equivalence check |
| 16 | `docs/progress/day_16_flowcharts.md` with Mermaid flowcharts for each program; max-of-three |
| 17 | Remove/erase for map and vector |
| 18 | A true chained-setter Named Parameter Idiom; replace the `0.0` sentinel with an overload or `std::optional` |
| 20 | A raw function pointer example; a callback API (`void for_each_result(const std::vector<double>&, const std::function<void(double)>&)`); reject unknown ops |
| 22 | A `Doxyfile` plus a `doxygen` custom target in CMake |
| 23 | Shadowing example; `static` local lifetime; a second function to show global sharing; unnamed namespace |
| 24 | Ship a genuinely buggy function plus a step-by-step GDB transcript in docs; `std::cerr` tracing; an `assert` example; a `Debug`+sanitizer preset |
| 25 | `CMakePresets.json`; fix `CMAKE_ARGC`; `CONFIGURE_DEPENDS`; testing enabled |
| 26 | Commit `.clang-format` and `.clang-tidy`; fix the `calculate_bmi` reference |
| 27 | Abstraction (pure virtual interface), inheritance, polymorphism (`virtual` + override + virtual destructor) |
| 28 | `explicit` ctor, invariant checks (throw on negative), destructor demo, exercise `withdraw` |
| 29 | Integrate one header-only library (e.g. nlohmann/json) and one compiled library (e.g. fmt) via `FetchContent` or `vcpkg.json` |

13. **Add the missing headers:**
- `<stdexcept>` in Days 12, 20, 21 and 22;
- `<cmath>` in `tests/test_day_07.cpp`;
- `<cctype>` in `tests/test_day_03.cpp`.

### P4: Syllabus and documentation sync
14. **Create `syllabus.md`** with the schema you intended (`Day | Topic | Key Learnings/Deliverables | Topic Level | Status of covered`). Make it the single source of truth and have README link to it instead of duplicating the table.
15. **Reconcile Days 24–29.** Either finish them (P3) and mark them ✅, or set the docs' `**Status:**` back to "In progress". Today README says Planned while the docs say Completed.
16. **Remove doc over-claims.** Rewrite the "Code Highlights" sections to match the actual code, or build the missing features. Specifically:

    | Day | Over-claim |
    |---|---|
    | 08 | login checker |
    | 09 | safe division, range check |
    | 10 | dice roller, password generator |
    | 11 | custom exception, rethrow, bounds checking |
    | 12 | factorial, rectangle |
    | 13 | factorial, break/continue |
    | 14 | Allman style, dangling-else example |
    | 15 | validation loop, sum calculator |
    | 16 | max of three |
    | 17 | remove |
    | 20 | callback system, wrong path |
    | 22 | poor-vs-good comparison |
    | 24 | buggy functions |
    | 28 | destructors |
17. Fix the README tree names (`day_01_variables.md`, `day_20_returning_function`) and remove the leftover chat-assistant HTML comment at README.md:335. Fix the "Next Day Preview" lines in Days 10–12.
18. Make `procpp.sh` cross-platform by listing `build/day_*` executables without assuming `.exe`.

---

*Generated by an automated audit. Every item marked "(verified)" was reproduced by building the project and running the named executable or test with the input shown.*
