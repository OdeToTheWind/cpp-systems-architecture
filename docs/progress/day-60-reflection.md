# Day 60 – API Authentication Reflection

**Date:** 2026-05-12 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_60_api_auth/lesson.hpp`](../../src/day_60_api_auth/lesson.hpp) · **Tests:** [`tests/test_day_60.cpp`](../../tests/test_day_60.cpp) (7 tests)

## Scenario
A *shipping-rate aggregator* that queries three carriers, each with a different authentication scheme: an API key header, a short-lived Bearer token that must be refreshed, and HTTP Basic auth. Credentials come from environment variables, are wrapped so they cannot be printed by accident, and are redacted from every log line.

## Syllabus deliverables
> API keys, Bearer tokens, Basic auth with Base64, secrets from environment variables, redaction

| Deliverable | Implemented in |
|---|---|
| ✅ a secret type that refuses to print itself | `Secret` |
| ✅ loading every credential from the environment at once | `load_credentials` |
| ✅ Base64 for HTTP Basic auth | `basic_auth_header` |
| ✅ a Bearer token cache that refreshes before expiry | `TokenCache::header` |
| ✅ one Authorization header per carrier scheme | `auth_headers` |
| ✅ removing secrets from log lines | `redact` |

## Key learnings
- APIs authenticate with a key header, a short-lived Bearer token or HTTP Basic auth; each just ends up as a request header.
- Basic auth is Base64 of "user:password" – an encoding, not encryption – so it is only safe over TLS.
- A token cache with an injectable clock refreshes a little before expiry, so requests never race an expiring token.
- Secrets live in environment variables, are wrapped in a type that prints ***, and are redacted from logs in both raw and Base64 forms.

## Pitfalls I hit (and how I fixed them)
- Missing environment variables were reported one at a time; the loader now lists every gap in one message.
- The Basic auth header leaked the password in logs because only the raw secret was redacted, not its Base64 form.
- Refreshing exactly at expiry caused occasional 401s; the cache now refreshes 30 seconds early.

## Run it
```bash
./procpp.sh 60                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_60_api_auth       # the interactive demo
ctest --test-dir build -R test_day_60 --output-on-failure
```

## Next step
- Day 61 automates notifications from health checks.
