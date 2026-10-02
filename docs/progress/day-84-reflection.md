# Day 84 – Data Pipeline / ETL Reflection

**Date:** 2026-06-05 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_84_etl/lesson.hpp`](../../src/day_84_etl/lesson.hpp) · **Tests:** [`tests/test_day_84.cpp`](../../tests/test_day_84.cpp) (7 tests)

## Scenario
The *nightly order import* of an online shop that sells through three marketplaces. Each marketplace exports a CSV in its own quirks; the pipeline extracts rows, transforms them into one clean order format (dates checked, money in integer cents, everything converted to EUR), loads them into the warehouse table without duplicates, and puts every bad row into a quarantine with the reason – so one broken line never stops the whole import.

## Syllabus deliverables
> Extract, transform and load stages, validation with quarantine, run summaries

| Deliverable | Implemented in |
|---|---|
| ✅ extracting CSV rows by header name | `extract` |
| ✅ transforming one raw row into a clean order or a rejection | `transform` |
| ✅ an idempotent load keyed by order id | `Warehouse::load` |
| ✅ the whole pipeline with quarantine | `run_pipeline` |
| ✅ a run summary for the morning report | `RunSummary::report` |

## Key learnings
- Separating extract, transform and load keeps each stage small and testable on its own.
- Bad rows go to a quarantine with their line number and reason, so one broken line never stops the import.
- Money is parsed straight into integer cents and converted with integer arithmetic and explicit rounding.
- Keying the load on (source, order id) makes re-running a file idempotent, and corrected re-exports update rows instead of duplicating them.

## Pitfalls I hit (and how I fixed them)
- Columns were read by position until one marketplace reordered its export; headers now map fields by name.
- "2026-02-30" passed a regex-style check; dates are now validated against the real calendar, including leap years.
- A quoted comma split a field in two until the CSV parser handled quotes and doubled quotes.

## Run it
```bash
./procpp.sh 84                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_84_etl       # the interactive demo
ctest --test-dir build -R test_day_84 --output-on-failure
```

## Next step
- Day 85 processes many files in parallel.
