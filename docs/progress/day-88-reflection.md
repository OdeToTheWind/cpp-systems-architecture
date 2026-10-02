# Day 88 – Automated Report Generator Reflection

**Date:** 2026-06-09 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_88_reports/lesson.hpp`](../../src/day_88_reports/lesson.hpp) · **Tests:** [`tests/test_day_88.cpp`](../../tests/test_day_88.cpp) (7 tests)

## Scenario
The *monthly billing report of a freelance design studio*. Logged hours become one report per month: amounts are integer cents (never floating point), VAT is rounded once per invoice line by a documented rule, and the same data is rendered as an HTML page through a small template engine and as a CSV for the accountant – both safely escaped.

## Syllabus deliverables
> Aggregation with exact integer money, templated HTML, CSV export, escaping

| Deliverable | Implemented in |
|---|---|
| ✅ money as integer cents with explicit rounding | `Money` |
| ✅ HTML escaping for untrusted text | `html_escape` |
| ✅ a template engine with escaped values and loops | `render_template` |
| ✅ CSV fields safe against injection | `csv_field` |
| ✅ building the report data once for both outputs | `build_report` |

## Key learnings
- Money lives in integer cents; only division needs a rounding rule, and it should be written down (half away from zero).
- Rounding per line and summing the rounded lines makes the report add up exactly as printed.
- A template engine that escapes by default and needs an explicit marker ({{&x}}) for raw HTML makes the safe path the easy one.
- CSV needs quoting for commas, quotes and newlines, and a leading ' to stop =, +, - or @ being run as spreadsheet formulas.

## Pitfalls I hit (and how I fixed them)
- Totals computed from unrounded values differed from the sum of the printed lines by a cent.
- A client named "Bolt <Ltd>" broke the HTML table until values were escaped.
- An explicit constructor with a default argument made `Entry e{}` fail to compile; Money now has a separate non-explicit default constructor.

## Run it
```bash
./procpp.sh 88                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_88_reports       # the interactive demo
ctest --test-dir build -R test_day_88 --output-on-failure
```

## Next step
- Day 89 runs jobs on a schedule.
