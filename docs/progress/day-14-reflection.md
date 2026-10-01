# Day 14 – Code Blocks and Indentation Reflection

**Date:** 2026-03-27 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_14_code_blocks/lesson.hpp`](../../src/day_14_code_blocks/lesson.hpp) · **Tests:** [`tests/test_day_14.cpp`](../../tests/test_day_14.cpp) (8 tests)

## Scenario
A *snippet checker for a coding bootcamp*. Students paste C++ snippets; the checker finds unbalanced brackets with line numbers, flags `if`/`else`/loops without braces (the dangling-else trap), reports mixed or odd indentation, and re-indents the code.

## Syllabus deliverables
> Block scope, braces for every branch, the dangling-else trap, consistent indentation style

| Deliverable | Implemented in |
|---|---|
| ✅ block scope ends an object's lifetime at the closing brace | `block_scope_demo` |
| ✅ matching brackets with a stack | `check_brackets` |
| ✅ flagging control statements without braces | `find_unbraced` |
| ✅ else binds to the nearest if (dangling else) | `dangling_else_unbraced` |
| ✅ measuring indentation consistency | `indentation_report` |
| ✅ re-indenting by brace depth | `reindent` |

## Key learnings
- Every pair of braces opens a scope, and objects declared inside are destroyed at its closing brace, in reverse order of construction.
- An `else` always binds to the nearest unmatched `if`, whatever the indentation says – braces on every branch make code mean what it looks like.
- Compilers can spot misleading layouts (`-Wdangling-else`, `-Wmisleading-indentation`); keeping warnings on catches them.
- Matching brackets is a stack problem: push on open, pop and compare on close, and report what is left at the end.

## Pitfalls I hit (and how I fixed them)
- The bracket checker reported errors inside string literals and comments until it learned to skip them.
- Re-indenting dedented the line *after* a closing brace instead of the brace itself; the depth is now lowered before a line starting with `}` is printed.
- The dangling-else example would not build with `-Werror`, so the warning is silenced for that one function, with a comment saying why.

## Run it
```bash
./procpp.sh 14                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_14_code_blocks       # the interactive demo
ctest --test-dir build -R test_day_14 --output-on-failure
```

## Next step
- Day 15 writes loops whose length is not known in advance – while and do-while.
