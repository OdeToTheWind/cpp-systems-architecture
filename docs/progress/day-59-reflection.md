# Day 59 – Query Parameters, Headers & Payloads Reflection

**Date:** 2026-05-11 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_59_request_payloads/lesson.hpp`](../../src/day_59_request_payloads/lesson.hpp) · **Tests:** [`tests/test_day_59.cpp`](../../tests/test_day_59.cpp) (8 tests)

## Scenario
A *hotel-search API client*. What matters is exactly what goes on the wire, so every request is first *prepared* offline – URL with percent-encoded query, headers, form or JSON body with the right Content-Type and Content-Length – and can be inspected byte by byte before anything would be sent.

## Syllabus deliverables
> Percent-encoding, query strings, custom headers, form and JSON request bodies

| Deliverable | Implemented in |
|---|---|
| ✅ RFC 3986 percent-encoding | `percent_encode` |
| ✅ query strings with repeated keys | `query_string` |
| ✅ case-insensitive custom headers | `Headers` |
| ✅ a form-encoded body | `RequestBuilder::form` |
| ✅ a JSON body with content headers set automatically | `RequestBuilder::json` |
| ✅ the prepared request as wire text | `PreparedRequest::wire` |

## Key learnings
- Percent-encoding leaves only unreserved characters untouched and encodes every other UTF-8 byte as %XX.
- Query strings keep their order and may repeat a key, so they are a list of pairs rather than a map.
- Form bodies use the same encoding with '+' for spaces; JSON bodies need Content-Type: application/json, and both need an exact Content-Length.
- Preparing a request offline and printing its wire format makes every byte checkable before anything is sent.

## Pitfalls I hit (and how I fixed them)
- A city name with a space broke the URL until values were percent-encoded.
- Header values containing CR/LF could inject extra headers; `Headers::set` now rejects line breaks.
- Logging the prepared request printed the API key; the wire text can now mask secret headers.

## Run it
```bash
./procpp.sh 59                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_59_request_payloads       # the interactive demo
ctest --test-dir build -R test_day_59 --output-on-failure
```

## Next step
- Day 60 adds authentication: API keys, bearer tokens and Basic auth.
