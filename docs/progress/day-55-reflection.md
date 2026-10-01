# Day 55 – Hosting C++ Online Reflection

**Date:** 2026-05-07 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_55_hosting/lesson.hpp`](../../src/day_55_hosting/lesson.hpp) · **Tests:** [`tests/test_day_55.cpp`](../../tests/test_day_55.cpp) (8 tests)

## Scenario
A *"Word of the Day" web service* for a language school, written so it can be hosted anywhere: as a CGI program behind Apache or nginx today, or behind an embedded HTTP server later. The request comes from CGI environment variables, a router picks the handler, the response is written in CGI format, and settings come from the environment with safe defaults.

## Syllabus deliverables
> Request/response handlers, CGI-style environment parsing, routing, deployment-ready configuration

| Deliverable | Implemented in |
|---|---|
| ✅ a request built from CGI environment variables | `request_from_cgi` |
| ✅ decoding the query string | `parse_query` |
| ✅ a router mapping method and path to handlers | `Router` |
| ✅ responses rendered in CGI format | `render_cgi` |
| ✅ configuration from the environment with defaults | `load_config` |

## Key learnings
- CGI is the simplest hosting model: the web server puts the request in environment variables (REQUEST_METHOD, PATH_INFO, QUERY_STRING) and reads the response from standard output.
- Separating request parsing, routing, handlers and response rendering means the same application can later run inside an embedded HTTP server unchanged.
- A router that distinguishes 404 (no such path) from 405 (path exists, wrong method) gives clients precise answers.
- Deployment settings belong in environment variables with safe defaults, validated once at start-up so a typo fails fast.

## Pitfalls I hit (and how I fixed them)
- A handler exception printed the database error to the visitor; the router now returns a generic 500 and keeps details out of the response.
- `day=-1` parsed as a huge unsigned number; the handler now rejects anything that starts with a minus sign.
- Query values with `+` and `%20` arrived still encoded until `parse_query` decoded them.

## Run it
```bash
./procpp.sh 55                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_55_hosting       # the interactive demo
ctest --test-dir build -R test_day_55 --output-on-failure
```

## Next step
- Day 56 builds a real command-line tool with flags, options and exit codes.
