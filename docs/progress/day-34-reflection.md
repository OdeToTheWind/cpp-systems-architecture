# Day 34 – Optional, Required & Default Parameters Reflection

**Date:** 2026-04-16 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_34_optional_parameters/lesson.hpp`](../../src/day_34_optional_parameters/lesson.hpp) · **Tests:** [`tests/test_day_34.cpp`](../../tests/test_day_34.cpp) (7 tests)

## Scenario
A *continuous-integration job scheduler*. A job always needs a name and a command; everything else – timeout, branch filter, retries, build matrix – is optional, and "not given" must stay distinguishable from "given as zero".

## Syllabus deliverables
> std::optional parameters, overload sets, builder objects, parameter ordering rules

| Deliverable | Implemented in |
|---|---|
| ✅ required parameters first, defaults last | `schedule_job` |
| ✅ std::optional tells 'not given' apart from 'given as 0' | `effective_retries` |
| ✅ an overload for a different shape of input | `schedule_matrix` |
| ✅ a builder for many optional settings | `JobBuilder` |
| ✅ validation of the combined settings | `validate` |

## Key learnings
- Parameters with defaults must come after all required parameters, because arguments are matched strictly from the left.
- `std::optional<int>` distinguishes 'not given' (use the project default) from 'given as 0' (really no retries) – a plain default value cannot.
- An overload is the right tool when a different *shape* of input is accepted, such as a whole build matrix instead of one job.
- A builder takes required values in its constructor and optional ones through named setters, then validates once in `build()`.

## Pitfalls I hit (and how I fixed them)
- Retries defaulted to 0, so 'use the default' and 'no retries' could not be told apart; `std::optional<int>` fixed it.
- To set only the retries, every earlier optional argument had to be repeated – the builder avoids that.
- Validation lived in `schedule_job` only, so jobs created by the builder skipped it; both paths now call `validate`.

## Run it
```bash
./procpp.sh 34                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_34_optional_parameters       # the interactive demo
ctest --test-dir build -R test_day_34 --output-on-failure
```

## Next step
- Day 35 lets objects react to events without knowing who triggers them.
