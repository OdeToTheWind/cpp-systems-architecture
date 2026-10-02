# Day 96 – Packaging a Real Tool Reflection

**Date:** 2026-06-17 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_96_release_tool/lesson.hpp`](../../src/day_96_release_tool/lesson.hpp) · **Tests:** [`tests/test_day_96.cpp`](../../tests/test_day_96.cpp) (7 tests)

## Scenario
Releasing *logslice*, a small command-line tool that cuts a time window out of log files. A release is more than a build: the version is defined once (day.cmake) and reaches the binary through a generated header; the usage text and the Markdown docs are generated from the same option table; the changelog is parsed and checked; and a pre-flight check refuses to ship if any of these disagree.

## Syllabus deliverables
> Single-sourced versions, generated usage docs, changelogs, release pre-flight checks

| Deliverable | Implemented in |
|---|---|
| ✅ one option table behind help text and docs | `OPTIONS` |
| ✅ usage docs generated as Markdown | `markdown_docs` |
| ✅ parsing a Keep a Changelog file | `parse_changelog` |
| ✅ checking that a release is a valid semantic-version bump | `valid_bump` |
| ✅ the pre-flight checklist that gates a release | `preflight` |

## Key learnings
- A version defined once in the build system and written into a generated header can never disagree with the binary.
- Help text and Markdown docs generated from one option table stay in sync; a test compares the committed USAGE.md with the generated text.
- A Keep a Changelog file is easy to parse and check: newest first, dated, with entries, and no leftovers under Unreleased.
- A pre-flight check lists every blocker before tagging a release instead of discovering them one by one.

## Pitfalls I hit (and how I fixed them)
- A release bumped 1.2.1 straight to 1.4.0; the check now requires a single semantic-version step.
- Line endings converted on checkout would make the committed docs differ from the generated text, so the comparison reads files in binary mode and .gitattributes fixes LF.
- The changelog said 1.3.0 while the binary still said 1.2.1 – exactly the mismatch the single source of truth removes.

## Run it
```bash
./procpp.sh 96                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_96_release_tool       # the interactive demo
ctest --test-dir build -R test_day_96 --output-on-failure
```

## Next step
- Day 97 automates a recurring chore with a bot.
