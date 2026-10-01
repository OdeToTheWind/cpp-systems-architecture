# Day 02 – String Manipulation Reflection

**Date:** 2026-03-15 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_02_strings/lesson.hpp`](../../src/day_02_strings/lesson.hpp) · **Tests:** [`tests/test_day_02.cpp`](../../tests/test_day_02.cpp) (9 tests)

## Scenario
A *conference badge printer* that turns messy sign-up rows such as `" ada LOVELACE ; analytical engines ltd "` into clean, centred, fixed-width badges.

## Syllabus deliverables
> std::string length, find, substr, concatenation, trimming and case-folding input, aligned formatting

| Deliverable | Implemented in |
|---|---|
| ✅ trimming whitespace with find\_first\_not\_of / find\_last\_not\_of | `trim` |
| ✅ case-folding with std::tolower / std::toupper | `to_title_case` |
| ✅ splitting with find and substr | `split_once` |
| ✅ searching repeatedly with find | `count_occurrences` |
| ✅ concatenation and aligned formatting | `make_badge` |
| ✅ comparing characters while ignoring case and punctuation | `is_palindrome` |

## Key learnings
- `find` returns `std::string::npos` when nothing matches, so every `find` result must be compared with `npos` before it is used as a position.
- `substr(pos, count)` takes a *length*, not an end position – `last - first + 1` is the length of the trimmed range.
- `std::tolower` and `std::isalpha` take an `int` that must be representable as `unsigned char`; casting `char` to `unsigned char` first avoids undefined behaviour for accented letters.
- `std::string_view` lets helpers such as `trim` accept literals, `std::string`s and slices without copying.

## Pitfalls I hit (and how I fixed them)
- The palindrome check used `isalpha`, so `"1a2"` counted as a palindrome; switching to `isalnum` keeps digits in the comparison.
- `centre` underflowed `width - size` when the text was longer than the badge; long text is now shortened with `...` first.
- Calling `substr` on a `npos` result from `find` threw `std::out_of_range`; `split_once` now returns `std::nullopt` when the delimiter is missing.

## Run it
```bash
./procpp.sh 2                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_02_strings       # the interactive demo
ctest --test-dir build -R test_day_02 --output-on-failure
```

## Next step
- Day 03 reads those rows from the console and recovers when the user types something unexpected.
