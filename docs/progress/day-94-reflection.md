# Day 94 – Data Validation & Cleaning Library Reflection

**Date:** 2026-06-15 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_94_validation/lesson.hpp`](../../src/day_94_validation/lesson.hpp) · **Tests:** [`tests/test_day_94.cpp`](../../tests/test_day_94.cpp) (7 tests)

## Scenario
The *order API of an event-ticketing site*. A request contains a buyer and a list of attendees. Small validators (string length, e-mail, integer range, one-of) are composed into a schema for the whole nested payload; validation reports *every* problem with a path such as `attendees[1].email`, and returns a normalised copy (trimmed text, lower-cased e-mails, "2" -> 2, defaults filled in) that the rest of the system can trust.

## Syllabus deliverables
> Composable validators, error paths, normalising messy input, custom exceptions

| Deliverable | Implemented in |
|---|---|
| ✅ a dynamic value for untrusted input | `Value` |
| ✅ validators that normalise as they check | `text` |
| ✅ composing validators in sequence | `all_of` |
| ✅ objects with required, optional and unknown fields | `object` |
| ✅ lists whose errors carry the item index | `list` |

## Key learnings
- Small validators (text, integer, e-mail, one-of) compose into schemas for whole nested payloads.
- Each validator returns a normalised value, so the rest of the system receives trimmed, lower-cased, typed data.
- Paths such as attendees[1].email tell an API client exactly which field to fix.
- Objects keep validating after the first bad field, so all problems are reported at once; unknown fields are rejected.

## Pitfalls I hit (and how I fixed them)
- all_of ran a custom check on an unparsed string and crashed; each step now receives the previous step's normalised output, and stops at the first failure.
- "2.5" was accepted as a whole number until fractional values were rejected explicitly.
- Silently dropping unknown fields hid client typos such as 'emial'; they are now errors.

## Run it
```bash
./procpp.sh 94                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_94_validation       # the interactive demo
ctest --test-dir build -R test_day_94 --output-on-failure
```

## Next step
- Day 95 compares a naive algorithm with a spatial index.
