# Day 72 – Design Patterns Reflection

**Date:** 2026-05-24 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_72_design_patterns/lesson.hpp`](../../src/day_72_design_patterns/lesson.hpp) · **Tests:** [`tests/test_day_72.cpp`](../../tests/test_day_72.cpp) (7 tests)

## Scenario
A *note-taking editor*. Notes are lists of lines that can be exported as plain text, Markdown or HTML (strategy), with exporters created by name from a registry (factory), extra behaviour such as line numbers or a word-count footer stacked on top (decorator), and every edit recorded as an object that can be undone and redone (command).

## Syllabus deliverables
> Strategy, factory, decorator and command patterns in modern C++

| Deliverable | Implemented in |
|---|---|
| ✅ interchangeable export strategies behind one interface | `Exporter` |
| ✅ a factory registry that creates exporters by name | `make_exporter` |
| ✅ decorators that add behaviour without subclassing | `WithLineNumbers` |
| ✅ edits as command objects | `Command` |
| ✅ undo and redo stacks | `History::undo` |

## Key learnings
- Strategy: one interface, several interchangeable algorithms chosen at run time.
- Factory: callers ask for an object by name; a registry of creator functions makes new formats pluggable.
- Decorator: a wrapper that implements the same interface adds behaviour and can be stacked in any order.
- Command: an edit as an object with execute and undo makes undo/redo stacks straightforward.

## Pitfalls I hit (and how I fixed them)
- Replacing "a" with "aa" looped forever until the search continued after the inserted text.
- A redo after a fresh edit replayed stale commands; a new command now clears the redo stack.
- A command that threw was still pushed onto the history; it is now recorded only after it succeeds.

## Run it
```bash
./procpp.sh 72                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_72_design_patterns       # the interactive demo
ctest --test-dir build -R test_day_72 --output-on-failure
```

## Next step
- Day 73 starts concurrency with threads and mutexes.
