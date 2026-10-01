# Day 56 – Command-Line Arguments Reflection

**Date:** 2026-05-08 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_56_command_line/lesson.hpp`](../../src/day_56_command_line/lesson.hpp) · **Tests:** [`tests/test_day_56.cpp`](../../tests/test_day_56.cpp) (8 tests)

## Scenario
`logscan`, a *log-search command-line tool* for an operations team. It takes flags (-i, -n, -c), options with values (-m 5 or --max=5), a pattern and any number of files, prints a usage message on --help or on a mistake, and returns grep-style exit codes so scripts can react: 0 = matches found, 1 = no match, 2 = usage or file error.

## Syllabus deliverables
> argc and argv, flags and options, positional arguments, usage messages and exit codes

| Deliverable | Implemented in |
|---|---|
| ✅ turning argc/argv into a vector of strings | `arguments_from` |
| ✅ parsing flags, options and positionals | `parse_args` |
| ✅ a usage message | `usage` |
| ✅ grep-style exit codes | `ExitCode` |
| ✅ executing the parsed command | `execute` |

## Key learnings
- `argv[0]` is the program name; the real arguments are `argv[1]` to `argv[argc - 1]`, easiest to handle as a `std::vector<std::string>`.
- Conventional CLIs accept combined short flags (`-in`), options with separate or attached values (`-m 5`, `--max=5`) and `--` to end option parsing.
- Exit codes are the CLI's return value to scripts – grep's convention is 0 for a match, 1 for none and 2 for errors.
- Usage errors go to standard error with a short usage message, so they never pollute output that is piped into another program.

## Pitfalls I hit (and how I fixed them)
- A pattern that started with '-' was taken as an unknown flag until `--` was supported.
- `-m` as the last argument read past the end of the vector; the parser now checks that a value follows.
- A missing file stopped the whole search; other files are now still searched and the exit code reports the error.

## Run it
```bash
./procpp.sh 56                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_56_command_line       # the interactive demo
ctest --test-dir build -R test_day_56 --output-on-failure
```

## Next step
- Day 57 moves from local tools to web APIs: REST and JSON.
