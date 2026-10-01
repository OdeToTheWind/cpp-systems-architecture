# Day 16 – Flowchart Programming Reflection

**Date:** 2026-03-29 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_16_flowchart_programming/lesson.hpp`](../../src/day_16_flowchart_programming/lesson.hpp) · **Tests:** [`tests/test_day_16.cpp`](../../tests/test_day_16.cpp) (6 tests)

## Scenario
An *airport check-in kiosk*. Each rule – the baggage fee, the boarding check and the queue the kiosk works through – is first drawn as a flowchart (kept in the code as Mermaid text) and then translated shape by shape into structured C++.

## Syllabus deliverables
> Translating decisions, processes and loops from a flowchart into structured C++

| Deliverable | Implemented in |
|---|---|
| ✅ flowcharts kept next to the code as Mermaid text | `flowchart` |
| ✅ decision diamonds become an if / else chain | `baggage_fee_cents` |
| ✅ early exits for terminal states | `boarding_check` |
| ✅ a loop arrow becomes a while loop over a queue | `process_queue` |

## Key learnings
- Each diamond in a flowchart becomes one condition, each rectangle a statement, and each arrow that points back up a loop.
- Terminal shapes (refuse, deny, issue) map naturally to early `return`s, which keeps the C++ as flat as the drawing.
- Keeping the flowchart as Mermaid text next to the code means it is reviewed and updated with the code instead of rotting in a wiki.
- Counting the diamonds gives the number of decisions – and a lower bound on the number of tests needed to cover every path.

## Pitfalls I hit (and how I fixed them)
- The first grading flowchart only bounded the top grade, so 150 points got a B; every path now starts with the range check.
- The prompt asked about a learner's permit while the variable was called `has_license`; names now match the flowchart labels.
- Boundary weights (exactly 23 kg and 32 kg) followed the wrong arrow until the comparisons were copied exactly from the diamonds.

## Run it
```bash
./procpp.sh 16                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_16_flowchart_programming       # the interactive demo
ctest --test-dir build -R test_day_16 --output-on-failure
```

## Next step
- Day 17 stores the passengers and bags in containers instead of separate variables.
