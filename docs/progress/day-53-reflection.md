# Day 53 – Sending Email (SMTP & MIME) Reflection

**Date:** 2026-05-05 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_53_email_smtp/lesson.hpp`](../../src/day_53_email_smtp/lesson.hpp) · **Tests:** [`tests/test_day_53.cpp`](../../tests/test_day_53.cpp) (8 tests)

## Scenario
A *housing co-op's monthly statement mailer*. Each member gets a MIME message with a plain-text and an HTML version plus a CSV attachment. Addresses are validated, header injection is impossible, and the SMTP conversation runs over an injectable transport – a real socket in production, a scripted fake in tests, or a dry run that only prints the dialogue.

## Syllabus deliverables
> Building MIME messages, address validation, the SMTP dialogue over an injectable transport, dry runs

| Deliverable | Implemented in |
|---|---|
| ✅ validating email addresses | `is_valid_address` |
| ✅ base64 for attachments | `base64_encode` |
| ✅ assembling a multipart MIME message | `build_mime` |
| ✅ the transport interface that hides the network | `Transport` |
| ✅ the SMTP client state machine | `SmtpClient::send` |
| ✅ a dry-run transport that only records | `DryRunTransport` |

## Key learnings
- A MIME message nests parts with boundaries: multipart/mixed holds a multipart/alternative (plain text and HTML) followed by base64-encoded attachments.
- SMTP is a line-based conversation of commands and three-digit reply codes; checking each code turns server refusals into clear errors.
- Putting the network behind a small `Transport` interface makes the whole protocol testable with a scripted fake server and usable as a dry run.
- Any header value built from user input must have CR and LF removed, or a crafted subject could inject extra headers.

## Pitfalls I hit (and how I fixed them)
- A body line starting with '.' ended the message early; SMTP dot-stuffing (sending '..' instead) fixed it.
- Base64 output without line breaks was rejected by a strict server; lines are now wrapped at 76 characters.
- A subject with `\r\nBcc:` could have added a hidden recipient – `header_safe` now replaces line breaks in header values.

## Run it
```bash
./procpp.sh 53                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_53_email_smtp       # the interactive demo
ctest --test-dir build -R test_day_53 --output-on-failure
```

## Next step
- Day 54 schedules deliveries with dates, times and time zones.
