# Day 81 – Regular Expressions Reflection

**Date:** 2026-06-02 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_81_regex/lesson.hpp`](../../src/day_81_regex/lesson.hpp) · **Tests:** [`tests/test_day_81.cpp`](../../tests/test_day_81.cpp) (7 tests)

## Scenario
A *support-ticket scrubber*. Before customer messages are copied into the bug tracker, personal data has to go: e-mail addresses, phone numbers, payment-card numbers and IP addresses are found with regular expressions and masked, while ticket IDs and log timestamps are parsed with capture groups.

## Syllabus deliverables
> std::regex matching and searching, capture groups, replacement, redacting sensitive data

| Deliverable | Implemented in |
|---|---|
| ✅ whole-string validation with regex\_match | `is_ticket_id` |
| ✅ capture groups that split a log line | `parse_log_line` |
| ✅ finding every match with sregex\_iterator | `find_emails` |
| ✅ replacement with a callback for each match | `replace_with` |
| ✅ redacting e-mails, phones, cards and IP addresses | `redact` |

## Key learnings
- regex_match requires the whole string to match; regex_search and sregex_iterator find matches inside text.
- Capture groups pull fields out of a match, and $1 refers to them in regex_replace.
- When a replacement depends on the match (keep the domain, check Luhn), iterate the matches and build the output yourself.
- Raw string literals R"(...)" keep backslashes readable in patterns.

## Pitfalls I hit (and how I fixed them)
- The phone pattern matched the middle of a 16-digit order number; ECMAScript regex has no look-behind, so the preceding character is captured and put back.
- Order numbers looked like card numbers until the Luhn checksum was required before masking.
- The pattern only knows North-American phone layouts – a test documents the gap instead of pretending it is covered.

## Run it
```bash
./procpp.sh 81                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_81_regex       # the interactive demo
ctest --test-dir build -R test_day_81 --output-on-failure
```

## Next step
- Day 82 builds a small storage engine.
