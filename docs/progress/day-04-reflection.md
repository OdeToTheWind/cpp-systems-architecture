# Day 04 – Variable Naming Rules Reflection

**Date:** 2026-03-17 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_04_variable_names/lesson.hpp`](../../src/day_04_variable_names/lesson.hpp) · **Tests:** [`tests/test_day_04.cpp`](../../tests/test_day_04.cpp) (9 tests)

## Scenario
A *naming review bot* for pull requests: it inspects every proposed identifier and reports errors (the name will not compile), warnings about reserved names, and style advice so that variables, functions, types and constants each follow one convention.

## Syllabus deliverables
> Legal identifiers, reserved words and reserved names, snake\_case and PascalCase conventions, intention-revealing names

| Deliverable | Implemented in |
|---|---|
| ✅ C++ keywords cannot be identifiers | `is_keyword` |
| ✅ the grammar of a legal identifier | `is_legal_identifier` |
| ✅ names reserved for the implementation | `is_reserved_name` |
| ✅ recognising snake\_case, PascalCase and UPPER\_SNAKE\_CASE | `detect_style` |
| ✅ converting camelCase to snake\_case | `to_snake_case` |
| ✅ spotting names that hide intent | `is_vague_name` |
| ✅ one review combining errors and advice | `review_name` |

## Key learnings
- An identifier starts with a letter or underscore, continues with letters, digits or underscores, and must not be one of the 92 C++20 keywords and alternative tokens.
- Names containing `__` anywhere, or starting with `_` followed by a capital letter, are reserved for the compiler and standard library – using them is undefined behaviour.
- A project convention matters more than which convention: here variables and functions use snake_case, types PascalCase and constants UPPER_SNAKE_CASE or kPascalCase.
- Hungarian prefixes (`strName`, `bDone`) duplicate what the type system already knows and go stale when the type changes.

## Pitfalls I hit (and how I fixed them)
- `to_snake_case("HTTPServer")` first produced `h_t_t_p_server`; an underscore is now inserted only at a lower-to-upper boundary or before the last capital of an acronym.
- The vague-name check flagged `string_builder` because it starts with `str`; a prefix now counts only when a capital letter follows it.
- `std::isupper` was called with a plain `char`, which is undefined for negative values – every classification now casts to `unsigned char`.

## Run it
```bash
./procpp.sh 4                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_04_variable_names       # the interactive demo
ctest --test-dir build -R test_day_04 --output-on-failure
```

## Next step
- Day 05 does arithmetic with well-named variables and learns where integer and floating-point maths differ.
