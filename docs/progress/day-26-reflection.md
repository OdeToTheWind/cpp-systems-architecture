# Day 26 – IDE Tips and Tricks Reflection

**Date:** 2026-04-08 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_26_ide_tips/lesson.hpp`](../../src/day_26_ide_tips/lesson.hpp) · **Tests:** [`tests/test_day_26.cpp`](../../tests/test_day_26.cpp) (7 tests)

## Scenario
A *pocket IDE coach*: a searchable shortcut cheat-sheet for VS Code, CLion and Visual Studio on each operating system, a live-template expander, and the refactorings an IDE performs – find references, go to definition and a token-aware rename that never touches strings, comments or longer names that merely contain the old one.

## Syllabus deliverables
> Navigation and refactoring workflows, keyboard shortcuts, token-aware rename, code templates

| Deliverable | Implemented in |
|---|---|
| ✅ a shortcut cheat-sheet per IDE and operating system | `search_shortcuts` |
| ✅ splitting code into tokens like an IDE does | `tokenize` |
| ✅ find all references | `find_references` |
| ✅ go to definition | `go_to_definition` |
| ✅ a rename that only changes whole identifier tokens | `rename_symbol` |
| ✅ expanding live templates with placeholders | `expand_template` |

## Key learnings
- IDE refactorings work on *tokens*, not raw text: a rename changes identifiers and leaves strings, comments and longer names alone.
- Go to definition, find all references and rename symbol have shortcuts in every IDE – learning those three saves more time than any other habit.
- Live templates (snippets) expand a short abbreviation into a correct skeleton, so loops and guards are never typed from scratch.
- A tokenizer only needs a handful of rules – identifiers, numbers, literals, comments and symbols – to support all of these features.

## Pitfalls I hit (and how I fixed them)
- A text search-and-replace renamed `total` inside `total_count` and inside a string; the token-aware rename fixed both.
- The tokenizer treated `9lives` as an identifier at first; a name starting with a digit is a number token, so the rename now rejects it.
- Shortcut tables copied from a blog mixed up macOS and Windows keys; the cheat-sheet now stores the OS explicitly.

## Run it
```bash
./procpp.sh 26                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_26_ide_tips       # the interactive demo
ctest --test-dir build -R test_day_26 --output-on-failure
```

## Next step
- Day 27 starts object-oriented design: interfaces, encapsulation and polymorphism.
