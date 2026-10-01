# Day 58 – HTTP Requests Reflection

**Date:** 2026-05-10 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_58_http_client/lesson.hpp`](../../src/day_58_http_client/lesson.hpp) · **Tests:** [`tests/test_day_58.cpp`](../../tests/test_day_58.cpp) (8 tests)

## Scenario
A *public-transport departures client* for a station display. It writes correct HTTP/1.1 requests, parses status lines, headers and both plain and chunked bodies, and retries timeouts and temporary server errors with exponential back-off – all over an injectable transport, so tests use canned responses and never wait or touch the network.

## Syllabus deliverables
> HTTP/1.1 request and response formats, parsing status lines and headers, timeouts and retries over an injectable transport

| Deliverable | Implemented in |
|---|---|
| ✅ writing an HTTP/1.1 request | `format_request` |
| ✅ parsing the status line and case-insensitive headers | `parse_response` |
| ✅ decoding a chunked body | `decode_chunked` |
| ✅ the transport interface and its failure types | `Transport` |
| ✅ retries with exponential back-off and Retry-After | `HttpClient::get` |

## Key learnings
- An HTTP/1.1 request is plain text: a request line, header lines ending in CRLF, a blank line, then an optional body.
- Header names are case-insensitive and the body length comes from Content-Length or chunked transfer encoding.
- Putting the network behind a `Transport` interface lets tests replay canned responses, including timeouts.
- Exponential back-off doubles the wait between retries, capped at a maximum, and a server's Retry-After takes precedence.

## Pitfalls I hit (and how I fixed them)
- Retrying a 404 just repeated the same error; only timeouts, connection failures and 5xx responses are retried now.
- Bodies longer than Content-Length leaked trailing bytes into the result until the body was cut to the declared size.
- Real sleeps made the tests slow; the sleep function is injected and the tests only record the delays.

## Run it
```bash
./procpp.sh 58                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_58_http_client       # the interactive demo
ctest --test-dir build -R test_day_58 --output-on-failure
```

## Next step
- Day 59 builds query strings, headers and request bodies.
