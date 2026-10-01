# Day 49 – Advanced Error Handling Reflection

**Date:** 2026-05-01 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_49_advanced_errors/lesson.hpp`](../../src/day_49_advanced_errors/lesson.hpp) · **Tests:** [`tests/test_day_49.cpp`](../../tests/test_day_49.cpp) (7 tests)

## Scenario
A *concert ticket booking service*. Business failures – sold out, seat taken, payment declined – form an exception hierarchy callers can catch as broadly or as precisely as they need; the low-level seat-code parser reports problems as std::error_code values from its own error category; and a small Result type is used where failure is an ordinary outcome.

## Syllabus deliverables
> Custom exception hierarchies, std::error\_code, result types, choosing exceptions vs error values

| Deliverable | Implemented in |
|---|---|
| ✅ the root of a custom exception hierarchy | `BookingError` |
| ✅ specific exceptions callers may catch precisely | `SeatTaken` |
| ✅ an error-code enum and its category | `seat_errc` |
| ✅ a Result type for expected failures | `Result` |
| ✅ the low-level parser returning error codes | `parse_seat` |
| ✅ the high-level API that throws | `BoxOffice::book` |

## Key learnings
- An exception hierarchy (`BookingError` with `SoldOut`, `SeatTaken`, `PaymentDeclined` below it) lets callers catch broadly or precisely, and lets specific exceptions carry extra data.
- `std::error_code` pairs an integer with a category; a custom `std::error_category` plus `is_error_code_enum` makes your own enum work like the standard ones.
- A Result type (value *or* error, like C++23 `std::expected`) suits failures that are an ordinary outcome, such as a mistyped seat code.
- `std::system_error` bridges the two worlds: it is an exception that carries an `error_code`.

## Pitfalls I hit (and how I fixed them)
- `return seat_errc::empty;` did not compile for `Result<Seat>`: enum → error_code → Result is two user-defined conversions, so `Result` got a constructor template for error-code enums.
- Catching `BookingError` before `SeatTaken` meant the specific handler never ran; the most specific handler must come first.
- A test string containing `??'` triggered a trigraph warning; the bad seat code in the test is now `#!`.

## Run it
```bash
./procpp.sh 49                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_49_advanced_errors       # the interactive demo
ctest --test-dir build -R test_day_49 --output-on-failure
```

## Next step
- Day 50 makes sure that when something does throw, no data is left half-changed.
