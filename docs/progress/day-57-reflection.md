# Day 57 – REST APIs & JSON Reflection

**Date:** 2026-05-09 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_57_rest_api/lesson.hpp`](../../src/day_57_rest_api/lesson.hpp) · **Tests:** [`tests/test_day_57.cpp`](../../tests/test_day_57.cpp) (8 tests)

## Scenario
A *library-book REST API*, simulated in memory. No network is involved: the goal is to understand what each HTTP method means, which status code each outcome deserves, and how JSON request and response bodies are produced and consumed by a resource-oriented API.

## Syllabus deliverables
> HTTP methods and their meaning, status codes, resource routing, JSON request and response bodies

| Deliverable | Implemented in |
|---|---|
| ✅ which methods are safe and which are idempotent | `method_semantics` |
| ✅ parsing a flat JSON object body | `parse_object` |
| ✅ writing JSON objects and arrays | `to_json` |
| ✅ routing /books and /books/{id} to handlers | `BookApi::handle` |
| ✅ choosing the right status code for every outcome | `BookApi::create` |

## Key learnings
- REST models data as resources addressed by URLs; the HTTP method says what to do with them (GET reads, POST creates, PUT replaces, PATCH updates, DELETE removes).
- Safe methods never change server state; idempotent methods can be repeated with the same effect, which is what makes retrying them safe.
- Status codes are part of the contract: 201 with a Location header for a create, 204 for an empty success, 400 for bad input, 404 for a missing resource, 405 for a wrong method.
- Simulating the API in memory keeps routing and status-code logic testable without sockets.

## Pitfalls I hit (and how I fixed them)
- Returning 200 for every outcome hid errors from clients; each branch now has its own status code.
- `std::stoi` accepted "2024abc" as a year; the parser now requires the whole field to be a number.
- Hand-written JSON forgot to escape quotes inside titles until `to_json` escaped every string.

## Run it
```bash
./procpp.sh 57                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_57_rest_api       # the interactive demo
ctest --test-dir build -R test_day_57 --output-on-failure
```

## Next step
- Day 58 writes and parses the HTTP messages themselves.
