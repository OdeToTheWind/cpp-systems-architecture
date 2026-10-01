# Day 27 – Object-Oriented Programming Basics Reflection

**Date:** 2026-04-09 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_27_oop_basics/lesson.hpp`](../../src/day_27_oop_basics/lesson.hpp) · **Tests:** [`tests/test_day_27.cpp`](../../tests/test_day_27.cpp) (7 tests)

## Scenario
A *museum ticket kiosk payment gateway*. Cards, prepaid wallets and gift vouchers are very different, yet the kiosk charges all of them through one abstract interface – and a card number never leaves its object except as `**** 1111`.

## Syllabus deliverables
> Encapsulation, abstraction with pure virtual interfaces, polymorphism, information hiding

| Deliverable | Implemented in |
|---|---|
| ✅ an abstract interface of pure virtual functions | `PaymentMethod` |
| ✅ information hiding: the card number never leaves the object | `CardPayment` |
| ✅ encapsulated state changed only through methods | `WalletPayment` |
| ✅ a third implementation with one-time state | `VoucherPayment` |
| ✅ polymorphism: one call, many behaviours | `Gateway::process` |
| ✅ validation hidden behind the interface | `luhn_valid` |

## Key learnings
- An abstract class made of pure virtual functions describes *what* every implementation can do and nothing about *how*.
- Polymorphism lets the gateway call `method.charge(cents)` once and get card, wallet or voucher behaviour through virtual dispatch.
- Information hiding means private data with no getter at all when the data must never leave the object – the card number only appears masked.
- A base class used through pointers needs a virtual destructor, or deleting a derived object through the base is undefined behaviour.

## Pitfalls I hit (and how I fixed them)
- Copying a `CardPayment` into a `PaymentMethod` variable sliced it into the base type; methods are now passed by reference and stored as `unique_ptr<PaymentMethod>`.
- A debug print of the whole card object leaked the number into logs; the number has no accessor now, only `describe()`.
- A typo in a card number was accepted until the Luhn check moved into the constructor.

## Run it
```bash
./procpp.sh 27                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_27_oop_basics       # the interactive demo
ctest --test-dir build -R test_day_27 --output-on-failure
```

## Next step
- Day 28 designs a class around an invariant that no caller can break.
