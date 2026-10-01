# Day 43 – Reading and Writing CSV Reflection

**Date:** 2026-04-25 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_43_csv/lesson.hpp`](../../src/day_43_csv/lesson.hpp) · **Tests:** [`tests/test_day_43.cpp`](../../tests/test_day_43.cpp) (8 tests)

## Scenario
A *charity fun-run registration import*. The web form exports a CSV in which names contain commas and quotes, some rows are broken and some values are impossible. The importer parses quoted fields correctly, reports every bad row with its line number instead of crashing, and writes a clean CSV for the timing company.

## Syllabus deliverables
> Parsing quoted CSV fields, validating rows, reporting bad rows, writing CSV safely

| Deliverable | Implemented in |
|---|---|
| ✅ a quote-aware CSV record parser (RFC 4180) | `read_record` |
| ✅ validating one row into a typed record | `validate_row` |
| ✅ importing everything, collecting bad rows with line numbers | `import_runners` |
| ✅ quoting fields only when necessary | `csv_escape` |
| ✅ writing a CSV that round-trips | `write_runners` |

## Key learnings
- CSV fields may be quoted; inside quotes, commas and line breaks are data and a doubled quote `""` is one literal quote (RFC 4180).
- Because a quoted field can contain a newline, one record may span several physical lines – reading line by line is not enough.
- An importer should validate every row, collect the errors with line numbers and keep going, so a user can fix the whole file at once.
- When writing, quote a field only if it contains a comma, a quote or a line break, and double the quotes inside it.

## Pitfalls I hit (and how I fixed them)
- Splitting on every comma broke names like `"Lovelace, Ada"` into two columns; the parser now tracks whether it is inside quotes.
- An unterminated quote made the reader swallow the rest of the file; it now throws with the line where the record started.
- Written names containing quotes could not be read back until `csv_escape` doubled them.

## Run it
```bash
./procpp.sh 43                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_43_csv       # the interactive demo
ctest --test-dir build -R test_day_43 --output-on-failure
```

## Next step
- Day 44 analyses tabular data once it is loaded.
