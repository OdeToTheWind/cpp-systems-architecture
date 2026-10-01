# Day 63 – Browser Automation Reflection

**Date:** 2026-05-15 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_63_browser_automation/lesson.hpp`](../../src/day_63_browser_automation/lesson.hpp) · **Tests:** [`tests/test_day_63.cpp`](../../tests/test_day_63.cpp) (7 tests)

## Scenario
A *checkout smoke test* for an online shop, run before every release. The test logs in, adds a book to the cart and checks the order confirmation. A real run would drive a browser through the W3C WebDriver protocol; here a fake browser with delayed elements stands in for it, so locators, explicit waits and page objects can be practised – and tested – offline.

## Syllabus deliverables
> WebDriver commands, locator strategies, explicit waits, the page-object pattern

| Deliverable | Implemented in |
|---|---|
| ✅ locators by id, CSS class and link text | `Locator` |
| ✅ WebDriver commands as W3C HTTP requests | `to_wire` |
| ✅ a fake browser that implements the driver interface | `FakeShop` |
| ✅ an explicit wait with a timeout and poll interval | `wait_until` |
| ✅ page objects that hide locators from the test | `LoginPage::login` |

## Key learnings
- WebDriver is an HTTP protocol: every command (navigate, find element, click, send keys) is a POST to the driver with a small JSON body.
- Locators should be stable: ids first, then classes, and link text when that is what the user sees.
- Explicit waits poll a condition with a timeout, so tests are as fast as the page allows and fail with a clear message.
- Page objects keep selectors in one place; tests read as user actions such as login, add and checkout.

## Pitfalls I hit (and how I fixed them)
- A fixed sleep made the test slow when the page was fast and flaky when it was slow; explicit waits replaced it.
- A click handler that navigated destroyed its own std::function while it was running – AddressSanitizer would flag it; the handler is now copied before the call.
- Element handles from the previous page were reused after navigation; the fake browser now reports them as stale, like a real one.

## Run it
```bash
./procpp.sh 63                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_63_browser_automation       # the interactive demo
ctest --test-dir build -R test_day_63 --output-on-failure
```

## Next step
- Phase 4 starts with Day 64: templates and generic programming.
