# Day 51 – Working with JSON Reflection

**Date:** 2026-05-03 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_51_json/lesson.hpp`](../../src/day_51_json/lesson.hpp) · **Tests:** [`tests/test_day_51.cpp`](../../tests/test_day_51.cpp) (8 tests)

## Scenario
A *smart-garden irrigation controller* that is configured with JSON and reports its status as JSON. The lesson builds the JSON support itself – a value model, a recursive-descent parser that reports the line and column of every syntax error, and a serialiser that escapes strings correctly – the same job nlohmann/json does in production.

## Syllabus deliverables
> JSON value model, recursive-descent parsing, serialisation with escaping, error positions

| Deliverable | Implemented in |
|---|---|
| ✅ a JSON value model built on std::variant | `Json` |
| ✅ a recursive-descent parser | `Parser` |
| ✅ syntax errors with line and column | `JsonError` |
| ✅ escaping strings for output | `escape` |
| ✅ compact and pretty serialisation | `dump` |
| ✅ reading typed settings from parsed JSON | `load_zones` |

## Key learnings
- A JSON value maps naturally onto `std::variant<nullptr_t, bool, double, string, Array, Object>`, with arrays and objects holding further values recursively.
- Recursive descent turns a grammar into code: one function per rule (value, object, array, string, number), each consuming input and calling the others.
- Keeping the byte position lets the parser compute the line and column of any error, which makes broken configuration files easy to fix.
- When writing JSON, quotes, backslashes and control characters must be escaped, and NaN or infinity cannot be represented at all.

## Pitfalls I hit (and how I fixed them)
- A malicious file with thousands of nested `[` overflowed the stack; the parser now stops at a maximum depth.
- Numbers were parsed with the user's locale, so `1.5` failed on a German system; parsing and printing now use the classic "C" locale.
- `std::map` for objects reordered keys alphabetically, surprising users who diffed their config; objects now keep insertion order.

## Run it
```bash
./procpp.sh 51                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_51_json       # the interactive demo
ctest --test-dir build -R test_day_51 --output-on-failure
```

## Next step
- Day 52 saves application state to disk safely between runs.
