# Day 77 – Logging & Configuration Reflection

**Date:** 2026-05-29 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_77_logging_config/lesson.hpp`](../../src/day_77_logging_config/lesson.hpp) · **Tests:** [`tests/test_day_77.cpp`](../../tests/test_day_77.cpp) (7 tests)

## Scenario
A *payment gateway* that must be tuned per environment without recompiling. Settings come from an INI file and can be overridden by environment variables in production; log messages have levels, go through a formatter (human text or key=value for log shippers) and are written to any number of sinks, each with its own minimum level.

## Syllabus deliverables
> Log levels, formatters and sinks, INI-style configuration, environment overrides

| Deliverable | Implemented in |
|---|---|
| ✅ ordered log levels parsed from text | `parse_level` |
| ✅ text and key=value formatters | `Formatter` |
| ✅ sinks with their own minimum level | `Sink` |
| ✅ a logger that fans records out to sinks | `Logger::log` |
| ✅ parsing INI sections, keys and comments | `parse_ini` |
| ✅ environment variables overriding file settings | `Config::apply_env` |

## Key learnings
- Log levels are ordered, so each sink can choose its own threshold: everything to a debug file, only errors to the on-call channel.
- Separating formatters from sinks means any output (console, file, memory) can use any format (text, key=value).
- An INI file with [sections] keeps configuration readable; settings are addressed as section.key.
- Environment variables override file values, so one file works in every environment and secrets stay out of it.

## Pitfalls I hit (and how I fixed them)
- An inline "; comment" after a value became part of the value and broke integer parsing; inline comments now need whitespace before them.
- `std::stoi("8x")` returned 8; typed getters now require the whole value to be used.
- Timestamps made log tests flaky until the clock was injected.

## Run it
```bash
./procpp.sh 77                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_77_logging_config       # the interactive demo
ctest --test-dir build -R test_day_77 --output-on-failure
```

## Next step
- Day 78 looks more closely at how to test code with dependencies.
