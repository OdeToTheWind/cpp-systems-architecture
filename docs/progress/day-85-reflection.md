# Day 85 – Concurrent File Processor Reflection

**Date:** 2026-06-06 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_85_concurrent_files/lesson.hpp`](../../src/day_85_concurrent_files/lesson.hpp) · **Tests:** [`tests/test_day_85.cpp`](../../tests/test_day_85.cpp) (7 tests)

## Scenario
An *integrity checker for a photo archive*. Thousands of files sit on a NAS; once a week every file is checksummed in parallel and compared with the manifest from last time, so silent corruption, deleted files and unexpected new files are reported before the backups rotate. Results are deterministic – sorted by path – however the threads were scheduled.

## Syllabus deliverables
> Work distribution over a thread pool, checksums, per-item error reporting

| Deliverable | Implemented in |
|---|---|
| ✅ CRC-32 computed over a stream in chunks | `crc32` |
| ✅ a sorted, recursive file listing | `list_files` |
| ✅ workers claiming files through an atomic index | `checksum_all` |
| ✅ writing and reading a manifest | `read_manifest` |
| ✅ comparing the archive with its manifest | `verify` |

## Key learnings
- recursive_directory_iterator walks a tree; relative generic paths make manifests portable between systems.
- Streaming a file through the checksum in fixed-size chunks keeps memory flat for files of any size.
- Workers that claim the next index with fetch_add balance the load automatically, and writing into pre-sized slots needs no lock.
- Comparing against a manifest finds modified, missing and added files in one pass.

## Pitfalls I hit (and how I fixed them)
- Results came back in a different order on every run until each file wrote to its own slot instead of push_back under a lock.
- One unreadable file aborted the whole scan; errors are now recorded per file.
- The manifest listed itself and was always "modified"; it is skipped when listing files.

## Run it
```bash
./procpp.sh 85                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_85_concurrent_files       # the interactive demo
ctest --test-dir build -R test_day_85 --output-on-failure
```

## Next step
- Day 86 adds observability: logs, metrics and alerts.
