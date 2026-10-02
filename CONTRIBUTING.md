# Contributing to C++ Systems Architecture

Thank you for helping. A clearer sentence, an extra edge-case test or a fixed typo is
a real contribution, and every contribution makes the course better for the next learner.

By taking part you agree to follow the [Code of Conduct](CODE_OF_CONDUCT.md).

## Ways to contribute

| You want to… | Do this |
|---|---|
| Report a mistake in a lesson | Open a **Content error** issue: say which day, what's wrong and what you expected. |
| Report broken code or a failing test | Open a **Bug report** issue with the command, your OS, compiler, CMake version and the output. |
| Suggest a new exercise or scenario | Open an **Idea** issue before writing code, so we can agree on the scope first. |
| Fix something yourself | Fork the repository, make a branch, and open a pull request (see below). |

## Set up your environment

```bash
git clone https://github.com/<your-username>/cpp-systems-architecture.git
cd cpp-systems-architecture
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
./procpp.sh --check                  # must end with "All checks passed"
```

You need a C++20 compiler (GCC 13+, Clang 18+, Apple Clang 16+ or MSVC 2022), CMake 3.20+
and Python 3.10+ for the tooling. Nothing else: the test framework is a single header in
`include/cppm/` and no day downloads dependencies. CI runs GCC 13, GCC 14 and Clang 18 on
Linux, Apple Clang on macOS and MSVC on Windows, plus sanitizers and a coverage gate.

## How a day is built

Every day follows the same contract, and `tests/tooling/test_syllabus_sync.py` checks it.

```text
src/day_XX_<topic>/
├── lesson.hpp              # /** @file … Scenario: … */ · namespace cppm::dayXX · DELIVERABLES · code · run()
├── main.cpp                # calls cppm::dayXX::run(std::cin, std::cout)
├── *.cpp, subfolders       # optional: compiled into the day's static library
└── day.cmake               # optional: extra targets (a library, a generated header)
tests/test_day_XX.cpp                  # at least 5 TEST_CASEs; includes "day_XX_<topic>/lesson.hpp"
docs/progress/notes/day-XX.json        # hand-written learnings, pitfalls, next step
docs/progress/day-XX-reflection.md     # GENERATED, so never edit it by hand
docs/quiz/day-XX.json                  # 2 multiple-choice questions + bonus questions
```

Rules that keep the course trustworthy:

1. **One scenario per day.** The `Scenario:` paragraph in `lesson.hpp` must be unique across all 100 days.
2. **Every deliverable maps to code.** Each entry of `DELIVERABLES` names a function, class, member or
   constant that the tooling can find in the day's code.
3. **Tests teach.** Test real behaviour and edge cases. `CHECK(true)` and tests that only check that code
   runs are rejected. Never assert on wall-clock timings.
4. **No network, no stray files.** Inject transports, clocks and random engines; use `cppm::TempDir`
   for anything that touches the file system.
5. **Portable C++20.** The code must build warning-free with `-Wall -Wextra -Wpedantic -Wconversion
   -Wshadow` and under MSVC `/W4`. Avoid features that are missing from one of the CI standard
   libraries (for example `std::format`, chrono I/O, `std::jthread`) and seed randomness portably.
6. **Quizzes test this day's code.** Each `docs/quiz/day-XX.json` has exactly 2 multiple-choice
   questions (options A–D, one correct answer, an explanation that doesn't depend on option
   order) and at least one *discuss* and one *hands-on* bonus question, with no answer key.
   `tests/tooling/test_quiz.py` also keeps the correct letters balanced across the course.
7. **Generated files are generated.** After you change notes, lesson headers, `syllabus.md` or the tests,
   run `python3 scripts/build_reflections.py`. It rewrites the reflections and the generated blocks in the README.

## Style

* Formatting: `clang-format` with the repository's `.clang-format` (CI uses clang-format 18).
* Every public function has a short `///` comment that says what it does and, where useful, why.
* Keep lessons readable for learners: explicit names, short functions, a comment only where the *why* isn't obvious.
* Use British or American spelling consistently within a file.

## Pull requests

1. Create a branch: `git switch -c fix/day-18-default-argument-typo`.
2. Make one focused change. Smaller pull requests are reviewed faster.
3. Run `./procpp.sh --check` and make sure it passes and leaves no uncommitted generated files.
4. Write a commit message that says *what* changed and *why*.
5. Open the pull request and fill in the template.

A maintainer will review within a week. CI must be green before a merge.

## Licensing of contributions

By submitting a contribution you agree that it is licensed under the [MIT License](LICENSE),
the same terms as the rest of the project.
