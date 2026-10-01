# Day 52 – Local Persistence Reflection

**Date:** 2026-05-04 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_52_persistence/lesson.hpp`](../../src/day_52_persistence/lesson.hpp) · **Tests:** [`tests/test_day_52.cpp`](../../tests/test_day_52.cpp) (9 tests)

## Scenario
A *houseplant care app* that remembers every plant's watering interval and the day it was last watered between runs. Saving must never leave a half-written file behind, old save files from version 1 of the app must still load, and a corrupt file is set aside instead of crashing the app or silently losing it.

## Syllabus deliverables
> Saving and loading application state, atomic writes, schema versions, recovering from corrupt files

| Deliverable | Implemented in |
|---|---|
| ✅ the application state to persist | `GardenState` |
| ✅ serialising with a version header | `serialise` |
| ✅ parsing every supported schema version | `deserialise` |
| ✅ atomic save: write a temp file, then rename | `save_atomically` |
| ✅ loading with recovery from corrupt files | `load_or_recover` |

## Key learnings
- Writing to a temporary file and then renaming it over the real one is atomic on POSIX: readers see the complete old file or the complete new one, never a mix.
- A version header at the top of a save file lets newer program versions keep loading old files, with defaults for fields that did not exist yet.
- A file that cannot be parsed should be moved aside, not deleted – it is still the user's data and may be recoverable by hand.
- Loading a missing file is not an error: it simply means the first run.

## Pitfalls I hit (and how I fixed them)
- Writing the save file in place truncated it before the new contents arrived, so a crash mid-save lost every plant; the temp-file-and-rename approach fixed it.
- On Windows the corrupt file could not be renamed while it was still open; the stream is closed first.
- Version 1 files without an interval failed to load after the format changed until the parser learned both versions.

## Run it
```bash
./procpp.sh 52                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_52_persistence       # the interactive demo
ctest --test-dir build -R test_day_52 --output-on-failure
```

## Next step
- Day 53 sends email: MIME formatting and the SMTP conversation.
