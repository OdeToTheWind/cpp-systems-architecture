# Day 54 – Date and Time with chrono Reflection

**Date:** 2026-05-06 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_54_date_time/lesson.hpp`](../../src/day_54_date_time/lesson.hpp) · **Tests:** [`tests/test_day_54.cpp`](../../tests/test_day_54.cpp) (8 tests)

## Scenario
A *freight-forwarding delivery estimator*. Orders placed after the warehouse cut-off ship the next business day, transit counts business days only (no weekends, no public holidays), and the promised pick-up time is shown in every partner office's local time.

## Syllabus deliverables
> Durations and time points, calendar dates, business-day arithmetic, UTC offsets

| Deliverable | Implemented in |
|---|---|
| ✅ parsing and validating calendar dates | `parse_date` |
| ✅ weekday names from a date | `weekday_name` |
| ✅ business-day arithmetic that skips weekends and holidays | `add_business_days` |
| ✅ durations converted and formatted | `format_duration` |
| ✅ time points shifted by UTC offsets | `local_time` |
| ✅ a cut-off rule combining date and time of day | `ship_date` |

## Key learnings
- `std::chrono` separates durations (`minutes`, `hours`, `days`) from time points (`sys_time`), and the type system stops you from adding two time points.
- C++20 calendar types (`year_month_day`, `weekday`, `sys_days`) validate dates with `ok()` and convert to day counts for arithmetic.
- Business-day arithmetic is a loop over days that skips weekends and a set of holidays.
- A fixed UTC offset is just a duration added to a UTC time point; `floor<days>` then splits the result into a date and a time of day.

## Pitfalls I hit (and how I fixed them)
- Adding 30 days to January 31 by incrementing the month produced an invalid date; arithmetic is now done on `sys_days`.
- An order at exactly 15:00 shipped the same day; the cut-off check now uses `>=`.
- Converting a late-evening UTC time to UTC+05:30 kept the old date; deriving the date from the shifted time point fixed it.

## Run it
```bash
./procpp.sh 54                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_54_date_time       # the interactive demo
ctest --test-dir build -R test_day_54 --output-on-failure
```

## Next step
- Day 55 serves this kind of logic over the web.
