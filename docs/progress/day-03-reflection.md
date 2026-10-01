# Day 03 – Input & Output Streams Reflection

**Date:** 2026-03-16 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_03_input_output/lesson.hpp`](../../src/day_03_input_output/lesson.hpp) · **Tests:** [`tests/test_day_03.cpp`](../../tests/test_day_03.cpp) (9 tests)

## Scenario
A *workshop registration desk* that asks attendees questions in the console, re-asks after every invalid answer instead of crashing or looping forever, and prints a neatly aligned receipt with the ticket price.

## Syllabus deliverables
> Validated stream extraction, recovering from failed reads, buffer clearing, iomanip formatting

| Deliverable | Implemented in |
|---|---|
| ✅ validated extraction with range checks | `ask_int` |
| ✅ recovering from a failed read: clear() + ignore() | `recover_stream` |
| ✅ choosing from a fixed set of answers | `ask_choice` |
| ✅ iomanip formatting: setw, left/right, fixed, setprecision | `format_receipt` |
| ✅ a whole registration as one value | `Registration` |

## Key learnings
- When `std::cin >> value` fails, the stream sets `failbit` and every later read fails too, until `clear()` resets the flags.
- `clear()` alone is not enough: the bad characters are still in the buffer, so `ignore(numeric_limits<streamsize>::max(), '\n')` must discard the rest of the line.
- At end of input (`eof()`), retrying can never succeed – a read loop must stop instead of spinning forever.
- `std::setw` applies only to the next output item, while `std::fixed`, `std::setprecision` and `std::left` stay in effect until changed.

## Pitfalls I hit (and how I fixed them)
- Typing `abc` for the age made the program loop forever printing the prompt – `recover_stream` now clears the stream and returns false at end of input.
- Mixing `>>` and `std::getline` left an empty ticket answer; `ask_int` now discards the rest of the line after a successful read.
- Money was stored as `double` and printed `135.000000001`; prices are now whole cents in a `long long`.

## Run it
```bash
./procpp.sh 3                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_03_input_output       # the interactive demo
ctest --test-dir build -R test_day_03 --output-on-failure
```

## Next step
- Day 04 gives every value in these programs a name that explains itself.
