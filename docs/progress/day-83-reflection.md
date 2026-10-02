# Day 83 – Robust CLI Application Reflection

**Date:** 2026-06-04 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_83_robust_cli/lesson.hpp`](../../src/day_83_robust_cli/lesson.hpp) · **Tests:** [`tests/test_day_83.cpp`](../../tests/test_day_83.cpp) (7 tests)

## Scenario
`shelf`, a *reading-list tool* used from the terminal and from scripts. It has subcommands (add, list, done, stats), global options for verbosity and JSON output, `-c key=value` configuration overrides, a file-backed store, and documented exit codes so shell scripts can react to "not found" differently from "bad usage".

## Syllabus deliverables
> Subcommands, configuration files with overrides, verbosity flags, JSON output and exit codes

| Deliverable | Implemented in |
|---|---|
| ✅ documented exit codes for scripts | `ExitCode` |
| ✅ global options before a subcommand | `parse_command_line` |
| ✅ validated -c key=value configuration overrides | `Settings::apply` |
| ✅ one dispatcher for every subcommand | `execute` |
| ✅ machine-readable JSON output | `to_json` |

## Key learnings
- Global options before the subcommand and subcommand arguments after it keep a growing CLI predictable, like git or docker.
- Validating every -c key=value override before doing any work turns typos into clear usage errors instead of half-done runs.
- Results go to standard output and diagnostics to standard error, so `shelf --json list | jq` stays clean at any verbosity.
- Exit codes are a contract with scripts: 0 success, 1 failure, 2 usage, 3 not found.

## Pitfalls I hit (and how I fixed them)
- An unknown setting was silently ignored at first; it is now a usage error with the help text.
- A crash while saving could leave half a store file; the store is written to a temporary file and renamed.
- Quiet mode first suppressed JSON output too; -q now only removes chatter, never requested results.

## Run it
```bash
./procpp.sh 83                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_83_robust_cli       # the interactive demo
ctest --test-dir build -R test_day_83 --output-on-failure
```

## Next step
- Day 84 builds a data pipeline that survives bad input.
