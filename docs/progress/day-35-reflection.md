# Day 35 – Event Listeners & Callbacks Reflection

**Date:** 2026-04-17 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_35_event_listeners/lesson.hpp`](../../src/day_35_event_listeners/lesson.hpp) · **Tests:** [`tests/test_day_35.cpp`](../../tests/test_day_35.cpp) (7 tests)

## Scenario
A *smart-home hub*. Devices publish events – the doorbell rang, motion in the hallway, the smoke alarm went off – and any number of independent listeners react, without the devices knowing who is listening. Listeners can unsubscribe safely, even while an event is being delivered.

## Syllabus deliverables
> Observer pattern, subscription handles, unsubscribing safely, std::function listeners

| Deliverable | Implemented in |
|---|---|
| ✅ an event bus: the subject of the observer pattern | `EventBus` |
| ✅ std::function listeners for any callable | `EventBus::Listener` |
| ✅ subscribe returns a handle | `EventBus::subscribe` |
| ✅ unsubscribing safely, even during delivery | `EventBus::unsubscribe` |
| ✅ an RAII subscription that unsubscribes itself | `ScopedSubscription` |

## Key learnings
- The observer pattern decouples publishers from subscribers: a device publishes an event and never learns who reacted to it.
- `std::function<void(const Event&)>` lets listeners be lambdas, free functions or function objects.
- Returning a handle from `subscribe` gives callers a clean way to unsubscribe later.
- An RAII subscription object unsubscribes in its destructor, so a listener can never outlive the object it captured.

## Pitfalls I hit (and how I fixed them)
- A listener that unsubscribed itself during delivery erased an element from the vector being iterated; entries are now only marked inactive and removed after delivery.
- A listener that subscribed another listener during delivery caused the new one to run immediately; delivery now only visits listeners that existed when the event started.
- A lambda capturing a local by reference was still subscribed after the local died; `ScopedSubscription` ties the lifetimes together.

## Run it
```bash
./procpp.sh 35                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_35_event_listeners       # the interactive demo
ctest --test-dir build -R test_day_35 --output-on-failure
```

## Next step
- Day 36 models objects whose behaviour depends on the state they are in.
