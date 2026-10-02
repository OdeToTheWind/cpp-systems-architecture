# Day 97 – Automation Bot Suite Reflection

**Date:** 2026-06-18 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_97_automation_bot/lesson.hpp`](../../src/day_97_automation_bot/lesson.hpp) · **Tests:** [`tests/test_day_97.cpp`](../../tests/test_day_97.cpp) (7 tests)

## Scenario
A *price-drop bot*. Users put products on a wishlist with a target price (served by a small JSON API); the bot checks each product page on a schedule, scrapes the current price, retries flaky fetches with back-off, and notifies users when a price falls to their target – exactly once per price, however often the bot runs. Network, clock and notifications are all injected, so a whole day of bot activity is a fast, deterministic test.

## Syllabus deliverables
> Combining scraping, APIs, scheduling and notifications with deduplication and retries

| Deliverable | Implemented in |
|---|---|
| ✅ scraping a price from a product page | `extract_price` |
| ✅ reading wishlist subscriptions from a JSON API | `parse_wishlist` |
| ✅ retrying transient failures with exponential back-off | `with_retries` |
| ✅ notifying once per user, product and price | `Deduplicator::first_time` |
| ✅ a scheduled check of every product | `PriceBot::tick` |

## Key learnings
- A bot is a loop of small, separately testable parts: fetch, scrape, compare, notify.
- Only transient failures (timeouts, 503s) are retried, with exponential back-off; permanent errors fail fast.
- De-duplicating on (user, product, price) means a user hears about each new low price exactly once, however often the bot runs.
- Injecting the fetcher, the clock and the notifier lets a whole day of bot activity run in a test in milliseconds.

## Pitfalls I hit (and how I fixed them)
- Users were notified again every 30 minutes while the price stayed low; the deduplicator stopped the spam.
- A 404 was retried three times with growing delays; only TransientError is retried now.
- A failed fetch overwrote the last known price with nothing; the last good price is kept until a new one is read.

## Run it
```bash
./procpp.sh 97                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_97_automation_bot       # the interactive demo
ctest --test-dir build -R test_day_97 --output-on-failure
```

## Next step
- Day 98 simulates an epidemic numerically.
