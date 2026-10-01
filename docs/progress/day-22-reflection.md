# Day 22 – Documentation vs Comments Reflection

**Date:** 2026-04-04 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_22_documentation/lesson.hpp`](../../src/day_22_documentation/lesson.hpp) · **Tests:** [`tests/test_day_22.cpp`](../../tests/test_day_22.cpp) (8 tests)

## Scenario
A *kitchen unit-conversion library* that is documented the professional way – Doxygen comments describe what each function promises, ordinary comments explain why – plus a small documentation auditor that checks a header for undocumented functions.

## Syllabus deliverables
> Doxygen comments, why-comments vs what-comments, documenting preconditions and errors

| Deliverable | Implemented in |
|---|---|
| ✅ a fully documented function: @brief, @param, @return, @throws, @pre | `convert` |
| ✅ a why-comment explaining a non-obvious choice | `cups_to_millilitres` |
| ✅ documenting a precondition that is checked | `scale_recipe` |
| ✅ a documentation auditor that finds undocumented functions | `find_undocumented` |
| ✅ spotting comments that only repeat the code | `is_what_comment` |

## Key learnings
- Doxygen comments (`/** … */` or `///`) document the contract – parameters, return value, preconditions and exceptions – where callers look: in the header.
- Ordinary comments should explain *why* the code is the way it is; a comment that repeats *what* the code says adds noise and goes stale.
- Every `@throws` line is a promise that a test can check, which keeps the documentation honest.
- Running `doxygen` turns the header comments into browsable HTML, and its warnings list undocumented functions.

## Pitfalls I hit (and how I fixed them)
- The cup conversion used 250 ml until a why-comment had to justify it – US recipes use the 236.588 ml customary cup.
- A comment said 'returns grams' after the function had been changed to millilitres; the contract is now in the header and covered by a test.
- The documentation auditor flagged commented-out code as undocumented functions until it only looked at real declarations.

## Run it
```bash
./procpp.sh 22                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_22_documentation       # the interactive demo
ctest --test-dir build -R test_day_22 --output-on-failure
```

## Next step
- Day 23 examines where names are visible and how long objects live.
