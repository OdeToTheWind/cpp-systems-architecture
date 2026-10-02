# Day 100 – Portfolio Capstone: Production-ready C++ Tool Reflection

**Date:** 2026-06-21 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_100_portfolio_capstone/lesson.hpp`](../../src/day_100_portfolio_capstone/lesson.hpp) · **Tests:** [`tests/test_day_100.cpp`](../../tests/test_day_100.cpp) (7 tests)

## Scenario
`budget`, a *personal-finance command-line tool* – the portfolio piece that pulls the course together. It records income and spending in an append-only, checksummed ledger file, reads limits per category from an INI file with environment overrides, prints monthly reports with budget warnings and a CSV export, logs what it does, carries a single-sourced version from the build, and is covered module by module and end to end by tests.

## Syllabus deliverables
> A multi-module tool with a CLI, configuration, storage, reports, logging, packaging and a full test suite

| Deliverable | Implemented in |
|---|---|
| ✅ money parsing that refuses ambiguous input | `parse_money` |
| ✅ INI settings with environment overrides | `load_settings` |
| ✅ an append-only ledger that survives a torn write | `Ledger` |
| ✅ monthly totals with budget warnings | `build_month` |
| ✅ the command line: subcommands, logging and exit codes | `run_cli` |

## Key learnings
- A complete tool is many small modules – money, config, storage, reports, CLI – each with its own tests, plus end-to-end tests of the command line.
- Course techniques combine naturally: integer money (Day 88), INI with env overrides (Days 77, 91), an append-only checksummed file (Day 82), exit codes (Day 83) and a single-sourced version (Day 96).
- Running every test under warnings-as-errors, AddressSanitizer and two compilers catches bugs that the tests alone would miss.
- Writing the reflection and quiz for each day turned 100 days of code into notes that are worth rereading.

## Pitfalls I hit (and how I fixed them)
- `for (const auto& e : Ledger(file).entries())` iterated a destroyed temporary; GCC warned and AddressSanitizer proved it – the ledger is now a named variable.
- A negative amount for "spend" double-negated into income; amounts must be positive and the subcommand gives the direction.
- Validating the report month as text allowed "2026-13"; it is now checked as a real date.

## Run it
```bash
./procpp.sh 100                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_100_portfolio_capstone       # the interactive demo
ctest --test-dir build -R test_day_100 --output-on-failure
```

## Next step
- Next: contribute to an open-source C++ project and keep building.
