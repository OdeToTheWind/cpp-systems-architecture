# Day 42 – Working with Directories Reflection

**Date:** 2026-04-24 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_42_directories/lesson.hpp`](../../src/day_42_directories/lesson.hpp) · **Tests:** [`tests/test_day_42.cpp`](../../tests/test_day_42.cpp) (8 tests)

## Scenario
A *photo-shoot ingest tool*. After a shoot, a memory card is dumped into an inbox folder; the tool walks it, plans where every file belongs (raw/, jpeg/, video/, sidecar/, other/), avoids name collisions, prints the resulting tree – and refuses to touch anything outside the one folder it was given, so a typo can never damage the rest of the disk.

## Syllabus deliverables
> std::filesystem paths, creating and walking directories, filtering by extension, sandboxed operations

| Deliverable | Implemented in |
|---|---|
| ✅ a sandbox that rejects paths escaping its root | `Sandbox::resolve` |
| ✅ classifying files by extension | `category_for` |
| ✅ walking a directory tree recursively | `find_files` |
| ✅ planning moves with collision-free names | `plan_ingest` |
| ✅ creating folders and moving files | `apply_plan` |
| ✅ printing a directory tree | `tree` |

## Key learnings
- `std::filesystem::path` composes paths with `/` and handles separators portably; `lexically_normal` resolves `.` and `..` without touching the disk.
- `recursive_directory_iterator` walks a whole tree; sorting the results makes the behaviour deterministic across file systems.
- A sandbox that resolves every path relative to one root and rejects anything starting with `..` makes destructive tools safe to run.
- Planning moves first (a dry run) and executing them second lets the user review exactly what will happen.

## Pitfalls I hit (and how I fixed them)
- A relative path such as `inbox/../../etc` escaped the sandbox in the first draft; `lexically_relative` now detects the leading `..`.
- Two cameras both produced `IMG_1.jpg`, and the second move overwrote the first; destinations now get a numeric suffix when taken.
- Comparing a path ending in `/.` with the root failed because of the trailing separator; the sandbox now compares relative paths instead.

## Run it
```bash
./procpp.sh 42                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_42_directories       # the interactive demo
ctest --test-dir build -R test_day_42 --output-on-failure
```

## Next step
- Day 43 reads and writes the most common data file of all: CSV.
