# Day 82 – Building a Storage Engine Reflection

**Date:** 2026-06-03 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_82_storage_engine/lesson.hpp`](../../src/day_82_storage_engine/lesson.hpp) · **Tests:** [`tests/test_day_82.cpp`](../../tests/test_day_82.cpp) (7 tests)

## Scenario
The *session store* of a ticket-booking site – a small key-value database that must not lose a booking when the server crashes mid-write. Like Bitcask or a database's write-ahead log, it only ever appends records to a file, keeps an in-memory index from each key to the offset of its latest record, recovers from a torn final write, groups changes into all-or-nothing transactions, and compacts the log when it fills with stale records.

## Syllabus deliverables
> Append-only logs, in-memory indexes, crash recovery, compaction and transactions

| Deliverable | Implemented in |
|---|---|
| ✅ a checksummed record format | `encode_record` |
| ✅ appending records and indexing their offsets | `KvStore::set` |
| ✅ replaying the log on open and discarding a torn tail | `KvStore::open` |
| ✅ all-or-nothing transactions with a commit marker | `Transaction::commit` |
| ✅ rewriting only live keys and swapping files atomically | `KvStore::compact` |

## Key learnings
- An append-only log never overwrites data, so a crash can only damage the last record, never older ones.
- An in-memory index from each key to the offset of its latest record gives one seek per read.
- Per-record checksums and a newline terminator let recovery detect a torn tail, truncate it, and continue.
- A transaction's records count only once its commit marker is on disk; compaction writes live keys to a new file and renames it over the old one atomically.

## Pitfalls I hit (and how I fixed them)
- New records appended after a torn tail were unreadable until recovery truncated the file to the last good record.
- Values containing '|' or newlines broke the record format until they were escaped.
- Transaction ids restarted at 1 after a reopen and could merge with an old unfinished transaction; ids now continue from the highest one in the log.

## Run it
```bash
./procpp.sh 82                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_82_storage_engine       # the interactive demo
ctest --test-dir build -R test_day_82 --output-on-failure
```

## Next step
- Phase 5 starts with Day 83: a robust command-line application.
