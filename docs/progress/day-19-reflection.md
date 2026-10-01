# Day 19 – Pointers and References Reflection

**Date:** 2026-04-01 · **Level:** Beginner · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_19_pointers_references/lesson.hpp`](../../src/day_19_pointers_references/lesson.hpp) · **Tests:** [`tests/test_day_19.cpp`](../../tests/test_day_19.cpp) (8 tests)

## Scenario
A *hospital ward bed board*. Each bed is a slot in a fixed array; the board hands out pointers to free beds (or nullptr when the ward is full), moves patients between beds through references, and only ever reads through const pointers when it reports.

## Syllabus deliverables
> Address-of and dereference, references, pass by pointer vs reference, nullptr checks, const correctness

| Deliverable | Implemented in |
|---|---|
| ✅ & takes an address, \* dereferences it, a reference is an alias | `address_basics` |
| ✅ returning a pointer that may be nullptr | `Ward::find_free_bed` |
| ✅ a nullable pointer parameter checked against nullptr | `admit` |
| ✅ reference parameters for objects that must exist | `transfer` |
| ✅ const Bed\* for read-only access | `Ward::find_patient` |
| ✅ walking an array with pointer arithmetic | `count_occupied` |
| ✅ why pointers into a vector can dangle after push\_back | `vector_storage_moved` |

## Key learnings
- `&x` gives the address of `x`, `*p` reaches the object a pointer points at, and a reference is simply another name for an existing object.
- Use a reference when an argument must exist, and a pointer when 'nothing' (`nullptr`) is a valid answer – and then check it before using it.
- A const member function can only hand out pointers or references to const, so read-only access is enforced by the compiler.
- Pointer arithmetic is only valid inside one array: `begin() + 3` and `end() - begin()` work because the beds live in one `std::array`.

## Pitfalls I hit (and how I fixed them)
- A pointer into a `std::vector` dangled after `push_back` reallocated the storage – the beds now live in a fixed-size `std::array`.
- `transfer(bed, bed)` emptied the bed it was moving into; transferring a bed onto itself is now rejected by comparing addresses.
- Dereferencing the result of `find_free_bed()` on a full ward crashed – callers now check for `nullptr` first.

## Run it
```bash
./procpp.sh 19                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_19_pointers_references       # the interactive demo
ctest --test-dir build -R test_day_19 --output-on-failure
```

## Next step
- Day 20 returns functions themselves – through function pointers, lambdas and std::function.
