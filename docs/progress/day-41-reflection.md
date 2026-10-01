# Day 41 – File I/O with fstream Reflection

**Date:** 2026-04-23 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_41_file_io/lesson.hpp`](../../src/day_41_file_io/lesson.hpp) · **Tests:** [`tests/test_day_41.cpp`](../../tests/test_day_41.cpp) (7 tests)

## Scenario
A *flight data recorder for a hobby drone*. Events are appended to a text log a human can read, telemetry samples are stored in a compact binary file with a fixed, portable record layout, and every read checks for missing, unreadable or truncated files.

## Syllabus deliverables
> ifstream and ofstream, text and binary files, append mode, error checking, RAII file handling

| Deliverable | Implemented in |
|---|---|
| ✅ appending text lines with std::ios::app | `append_event` |
| ✅ reading a text file line by line with ifstream | `read_events` |
| ✅ writing fixed-size binary records portably | `write_samples` |
| ✅ reading binary records and detecting truncation | `read_samples` |
| ✅ checking every stream operation for errors | `measured_size` |

## Key learnings
- `std::ofstream` and `std::ifstream` close their file in the destructor, so RAII guarantees the file is flushed and released even when an exception is thrown.
- `std::ios::app` sends every write to the end of the file, keeping existing content; `std::ios::trunc` empties it first.
- Binary files should be written field by field in a defined byte order – dumping a struct with `write` copies padding and depends on the machine's endianness.
- After a read loop, `eof()` is the expected ending; `bad()` signals a real I/O failure, and `gcount()` reveals a partial final record.

## Pitfalls I hit (and how I fixed them)
- A helper called `file_size(path)` was ambiguous with `std::filesystem::file_size`, found through argument-dependent lookup; it is now `measured_size`.
- A crash during a flight left a half-written last record, which the reader turned into a garbage sample; `read_samples` now reports stray bytes as truncation.
- Opening a missing file 'worked' silently because the stream state was never checked; every open is now followed by `if (!stream)`.

## Run it
```bash
./procpp.sh 41                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_41_file_io       # the interactive demo
ctest --test-dir build -R test_day_41 --output-on-failure
```

## Next step
- Day 42 manages whole folders of files with std::filesystem.
