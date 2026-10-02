# Day 91 – Type-safe Configuration System Reflection

**Date:** 2026-06-12 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_91_typed_config/lesson.hpp`](../../src/day_91_typed_config/lesson.hpp) · **Tests:** [`tests/test_day_91.cpp`](../../tests/test_day_91.cpp) (7 tests)

## Scenario
The settings of a *food-delivery dispatch service*, read from environment variables in every deployment. Instead of scattered getenv calls and string-to-int conversions, one schema binds each variable to a typed struct member with a parser, a default and documentation. Loading reports *all* problems at once, secrets never appear in logs, and the schema can print its own .env.example.

## Syllabus deliverables
> Typed settings, parsing and validation from environment variables, collecting every error, secret redaction

| Deliverable | Implemented in |
|---|---|
| ✅ small parsers that throw readable messages | `parse_duration` |
| ✅ a schema binding variables to struct members | `Schema::field` |
| ✅ loading that collects every error | `Schema::load` |
| ✅ a secret type and redacted descriptions | `Schema::describe` |
| ✅ generating .env.example from the schema | `Schema::env_example` |

## Key learnings
- A schema that binds each environment variable to a typed struct member, with a parser, a default and help text, replaces scattered getenv calls.
- Pointers to members (`T Config::*`) let one generic loader write into any field of any config struct.
- Collecting every problem before failing means an operator fixes a deployment in one attempt, not five.
- The same schema prints a redacted description for logs and a documented .env.example for operators.

## Pitfalls I hit (and how I fixed them)
- The first loader stopped at the first bad variable, so fixing one error revealed the next only on the following deploy.
- A typo such as DISPATCH_REGION was silently ignored; unknown variables with the prefix are now reported.
- An error message echoed the rejected API key; secret fields never include their value in messages.

## Run it
```bash
./procpp.sh 91                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_91_typed_config       # the interactive demo
ctest --test-dir build -R test_day_91 --output-on-failure
```

## Next step
- Day 92 splits a real library into modules with a full test suite.
