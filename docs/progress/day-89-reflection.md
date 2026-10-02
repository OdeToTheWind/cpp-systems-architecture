# Day 89 – Background Task Scheduler Reflection

**Date:** 2026-06-10 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_89_scheduler/lesson.hpp`](../../src/day_89_scheduler/lesson.hpp) · **Tests:** [`tests/test_day_89.cpp`](../../tests/test_day_89.cpp) (7 tests)

## Scenario
The *maintenance scheduler of a SaaS back end*: refresh caches every 15 minutes, send usage reports hourly at :05, back up the database daily at 02:30. Schedules are written as short specs, time comes from an injectable clock so a whole week can be simulated in milliseconds, a job that runs too long is cut off by its timeout, and a job never overlaps with itself.

## Syllabus deliverables
> Schedule specifications, an injectable clock, timeouts, no overlapping runs

| Deliverable | Implemented in |
|---|---|
| ✅ parsing every / hourly / daily specs | `parse_schedule` |
| ✅ computing the next run strictly after a time | `next_after` |
| ✅ a clock the scheduler reads but never owns | `SimulatedClock` |
| ✅ cutting off runs that exceed their timeout | `Scheduler::run_due` |
| ✅ skipping a run while the previous one is still busy | `Scheduler::run_until` |

## Key learnings
- Short specs such as "every 15m", "hourly :05" and "daily 02:30" are parsed into a variant, and next_after computes the next run for each kind.
- Aligning interval schedules to multiples of the interval keeps them stable across restarts.
- With an injectable clock the scheduler jumps from event to event, so a week of scheduling is tested in milliseconds.
- A job still running when its next slot arrives is skipped, and a run past its timeout is cut off, so jobs never pile up.

## Pitfalls I hit (and how I fixed them)
- A schedule computed as "now + interval" drifted every time the process restarted.
- next_after returned the current time for a job due right now, so it ran twice; it is now strictly after.
- A throwing job stopped the whole scheduler until failures were caught and recorded per run.

## Run it
```bash
./procpp.sh 89                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_89_scheduler       # the interactive demo
ctest --test-dir build -R test_day_89 --output-on-failure
```

## Next step
- Day 90 processes files that do not fit in memory.
