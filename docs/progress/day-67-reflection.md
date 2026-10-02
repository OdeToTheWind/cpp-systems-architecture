# Day 67 – Smart Pointers & Ownership Reflection

**Date:** 2026-05-19 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_67_smart_pointers/lesson.hpp`](../../src/day_67_smart_pointers/lesson.hpp) · **Tests:** [`tests/test_day_67.cpp`](../../tests/test_day_67.cpp) (7 tests)

## Scenario
The object model of a *slide-deck editor*. A deck owns its slides outright, slides share image assets that are freed when no slide uses them, and grouped shapes point back to their parent group without keeping it alive. Every object counts itself, so tests can prove that nothing leaks and nothing is freed too early.

## Syllabus deliverables
> unique\_ptr, shared\_ptr and weak\_ptr, ownership transfer, breaking reference cycles

| Deliverable | Implemented in |
|---|---|
| ✅ exclusive ownership with unique\_ptr | `Deck::add_slide` |
| ✅ transferring ownership out of a container | `Deck::take_slide` |
| ✅ shared assets with a weak\_ptr cache | `AssetCache::load` |
| ✅ weak back-references that break cycles | `Shape::path` |
| ✅ what a shared\_ptr cycle does to lifetimes | `cycle_survives_scope` |

## Key learnings
- `std::unique_ptr` is the default for owned heap objects: one owner, no overhead, and ownership moves explicitly with `std::move`.
- `std::shared_ptr` is for genuinely shared lifetimes; `std::make_shared` allocates the object and its control block together.
- `std::weak_ptr` observes without owning – ideal for caches and back-references – and `lock()` safely checks whether the object still exists.
- Raw pointers and references remain fine for *viewing* an object someone else owns.

## Pitfalls I hit (and how I fixed them)
- Parent and child holding shared_ptrs to each other were never freed; the parent link is now a weak_ptr.
- A cache of shared_ptrs kept every image alive forever; it now stores weak_ptrs.
- `std::is_copy_constructible` reports true for a class holding `vector<unique_ptr>`, so the deck deletes its copy operations explicitly.

## Run it
```bash
./procpp.sh 67                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_67_smart_pointers       # the interactive demo
ctest --test-dir build -R test_day_67 --output-on-failure
```

## Next step
- Day 68 explains the moves that make unique_ptr work.
