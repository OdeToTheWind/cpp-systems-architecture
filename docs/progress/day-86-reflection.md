# Day 86 – Custom Logging & Monitoring Tool Reflection

**Date:** 2026-06-07 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_86_monitoring/lesson.hpp`](../../src/day_86_monitoring/lesson.hpp) · **Tests:** [`tests/test_day_86.cpp`](../../tests/test_day_86.cpp) (7 tests)

## Scenario
The *observability layer of a URL shortener*. Every request is logged as one JSON line (easy for log shippers), log files rotate by size so the disk never fills, request counts and latencies are exposed in the Prometheus text format, and alert rules fire only after a problem persists for several evaluations – and resolve on their own when it goes away.

## Syllabus deliverables
> Structured JSON logs, log rotation, metrics in Prometheus text format, alert thresholds

| Deliverable | Implemented in |
|---|---|
| ✅ one JSON object per log line | `json_log_line` |
| ✅ rotating log files by size | `RotatingFile::write` |
| ✅ counters, gauges and histograms in one registry | `Registry` |
| ✅ the Prometheus text exposition format | `Registry::expose` |
| ✅ alerts that fire after a duration and resolve | `Alerter::evaluate` |

## Key learnings
- One JSON object per line is the easiest log format for machines while staying readable for people.
- Size-based rotation renames app.log to app.log.1 and so on, deleting the oldest, so logs never fill the disk.
- Prometheus exposes counters, gauges and cumulative histograms as plain text with # HELP and # TYPE lines.
- Alert rules with a "for" duration fire only when a problem persists, and report a single RESOLVED when it clears.

## Pitfalls I hit (and how I fixed them)
- Histogram buckets came out as 100 before 50 because map keys sort as text; they are now sorted by numeric bound.
- A rotation in the middle of a write split a line between two files; rotation now happens before a line that would not fit.
- An alert computed on all-time totals stayed firing long after errors stopped; the rule now looks at the latest interval.

## Run it
```bash
./procpp.sh 86                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_86_monitoring       # the interactive demo
ctest --test-dir build -R test_day_86 --output-on-failure
```

## Next step
- Day 87 makes an application extensible with plugins.
