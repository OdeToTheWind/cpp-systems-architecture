# Day 61 – Notification Automation Reflection

**Date:** 2026-05-13 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_61_notifications/lesson.hpp`](../../src/day_61_notifications/lesson.hpp) · **Tests:** [`tests/test_day_61.cpp`](../../tests/test_day_61.cpp) (7 tests)

## Scenario
An *on-call alert bot* for a small web shop. Health readings (disk usage, response latency, queue depth) are compared with warning and critical thresholds; a state change produces a chat-webhook JSON payload, repeats are rate-limited per check, and recoveries are announced. Delivery goes through an injected sink, so a dry run prints what would be posted.

## Syllabus deliverables
> Health checks, alert thresholds, webhook payloads, rate limiting and dry-run delivery

| Deliverable | Implemented in |
|---|---|
| ✅ comparing a reading with warning and critical thresholds | `evaluate` |
| ✅ a chat-webhook JSON payload with escaped text | `webhook_payload` |
| ✅ a per-check cooldown that limits repeated alerts | `Cooldown::allow` |
| ✅ alerting on state changes and announcing recoveries | `Notifier::observe` |
| ✅ a dry-run sink that prints instead of posting | `dry_run_sink` |

## Key learnings
- Separate warning and critical thresholds give people time to act before an outage; the order warning <= critical is validated.
- Alerting on *state changes* (escalation and recovery) instead of every bad reading keeps the channel useful.
- A per-check cooldown rate-limits reminders while a problem persists, without hiding a new escalation.
- Injecting the delivery sink makes a dry run trivial: the same code prints the payload instead of posting it.

## Pitfalls I hit (and how I fixed them)
- Every reading above the threshold posted a message and flooded the channel until the cooldown was added.
- A recovery was never announced because OK readings were ignored; state is now tracked per check.
- A check name containing a quote produced invalid JSON until text was escaped.

## Run it
```bash
./procpp.sh 61                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_61_notifications       # the interactive demo
ctest --test-dir build -R test_day_61 --output-on-failure
```

## Next step
- Day 62 extracts data from web pages.
