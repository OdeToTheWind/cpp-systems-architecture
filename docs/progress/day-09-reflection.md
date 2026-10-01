# Day 09 – Logical Operators Reflection

**Date:** 2026-03-22 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_09_logical_operators/lesson.hpp`](../../src/day_09_logical_operators/lesson.hpp) · **Tests:** [`tests/test_day_09.cpp`](../../tests/test_day_09.cpp) (7 tests)

## Scenario
A *data-centre door controller* that decides whether a badge may open a door, explains every refusal, and proves with an evaluation trace exactly when C++ stops evaluating a condition.

## Syllabus deliverables
> &&, || and !, short-circuit evaluation, De Morgan's laws, named boolean conditions

| Deliverable | Implemented in |
|---|---|
| ✅ named boolean conditions combined with && and || | `can_enter` |
| ✅ ! to list what is missing | `denial_reasons` |
| ✅ short-circuit guards a null pointer | `badge_is_active` |
| ✅ tracing which operands are evaluated | `EvaluationTrace` |
| ✅ De Morgan's laws checked exhaustively | `de_morgan_holds` |

## Key learnings
- `&&` evaluates its right operand only if the left is true, and `||` only if the left is false – this short-circuit is guaranteed and ordered.
- Short-circuiting makes `ptr != nullptr && ptr->active` safe: the dereference never happens for a null pointer.
- De Morgan's laws, `!(a && b) == !a || !b` and `!(a || b) == !a && !b`, turn a negated policy into a list of individual reasons.
- Naming each sub-condition (`valid_badge`, `office_hours`) makes the final expression read like the security policy itself.

## Pitfalls I hit (and how I fixed them)
- The first version wrote `!badge->active && badge` and crashed on a null badge – operand order matters with short-circuiting.
- `!a && b` was read as `!(a && b)`; `!` binds tighter than `&&`, so parentheses now make every intent explicit.
- GCC flagged the De Morgan check as a self-comparison once it folded the expressions; computing each side into a named variable keeps the demonstration warning-free.

## Run it
```bash
./procpp.sh 9                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_09_logical_operators       # the interactive demo
ctest --test-dir build -R test_day_09 --output-on-failure
```

## Next step
- Day 10 adds randomness – and makes it reproducible so it can be tested.
