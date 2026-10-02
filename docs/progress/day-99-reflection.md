# Day 99 – Observability & Debugging Toolkit Reflection

**Date:** 2026-06-20 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_99_observability/lesson.hpp`](../../src/day_99_observability/lesson.hpp) · **Tests:** [`tests/test_day_99.cpp`](../../tests/test_day_99.cpp) (7 tests)

## Scenario
A *checkout service that fails once in a few thousand orders*, only in production. A debugger cannot be attached there, so the service carries its own instruments: spans that time each step of a request, a flight recorder of recent calls captured with std::source_location, watchpoints that log every change to a suspicious variable, and a crash reporter that turns an exception (with its nested causes) into a report with all of that context.

## Syllabus deliverables
> Crash reports, nested timed spans, call tracing, watchpoints on values

| Deliverable | Implemented in |
|---|---|
| ✅ a flight recorder of recent events | `FlightRecorder` |
| ✅ recording the caller's file and line automatically | `trace_call` |
| ✅ RAII spans that build a trace tree | `Span` |
| ✅ a variable that logs every change | `Watched` |
| ✅ crash reports with nested causes and context | `crash_report` |

## Key learnings
- std::source_location as a default argument captures the caller's file and line with no macros.
- RAII spans time each step of a request and form a tree; std::uncaught_exceptions tells a span whether it is ending because of an error.
- A watchpoint wrapper logs every change of a suspicious value and can trigger when a condition such as "stock < 0" becomes true.
- A crash report combines the exception chain (std::throw_with_nested), the flight recorder and the trace into one text an engineer can act on.

## Pitfalls I hit (and how I fixed them)
- function_name() is spelled differently by GCC, Clang and MSVC, so reports and tests use file and line only.
- Spans are already closed when the report is written after unwinding; the trace (with error statuses) shows where it happened instead of the open-span stack.
- Checking std::uncaught_exceptions() > 0 blamed spans created inside a catch handler; comparing with the count at construction fixed it.

## Run it
```bash
./procpp.sh 99                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_99_observability       # the interactive demo
ctest --test-dir build -R test_day_99 --output-on-failure
```

## Next step
- Day 100: the portfolio capstone.
