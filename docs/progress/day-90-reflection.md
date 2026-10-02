# Day 90 – Memory-efficient Large File Processor Reflection

**Date:** 2026-06-11 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_90_large_files/lesson.hpp`](../../src/day_90_large_files/lesson.hpp) · **Tests:** [`tests/test_day_90.cpp`](../../tests/test_day_90.cpp) (7 tests)

## Scenario
A *web-server access log* far larger than the memory of the machine that must analyse it. The file is read in fixed-size chunks, lines that straddle chunk boundaries are reassembled, statistics are computed in one streaming pass, and the log is sorted by response time with an external merge sort: sorted runs that fit in memory are written to temporary files and then merged with a priority queue.

## Syllabus deliverables
> Streaming in fixed-size chunks, line reassembly, external merge sort

| Deliverable | Implemented in |
|---|---|
| ✅ carrying partial lines from one chunk to the next | `LineAssembler::feed` |
| ✅ reading a stream in fixed-size chunks | `for_each_line` |
| ✅ one-pass statistics in constant memory | `summarise_log` |
| ✅ writing sorted runs that fit in memory | `write_sorted_runs` |
| ✅ a k-way merge with a priority queue | `merge_runs` |

## Key learnings
- Reading fixed-size chunks keeps memory constant; the only state carried between chunks is the unfinished last line.
- Statistics such as counts, sums and maxima can be computed in one streaming pass.
- External merge sort writes sorted runs that fit in memory and then merges them with a priority queue holding one line per run.
- Testing every chunk size from 1 byte upwards is a cheap way to catch boundary bugs.

## Pitfalls I hit (and how I fixed them)
- Lines split across chunks were counted twice until the assembler carried the partial line over.
- Windows \r\n endings left a trailing \r on every line; the assembler strips it.
- Equal keys from different runs came out in a different order than an in-memory stable sort; ties now favour the earlier run.

## Run it
```bash
./procpp.sh 90                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_90_large_files       # the interactive demo
ctest --test-dir build -R test_day_90 --output-on-failure
```

## Next step
- Day 91 validates configuration with types.
