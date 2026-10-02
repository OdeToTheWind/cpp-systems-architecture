# Day 93 – Network Service Core Reflection

**Date:** 2026-06-14 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_93_network_service/lesson.hpp`](../../src/day_93_network_service/lesson.hpp) · **Tests:** [`tests/test_day_93.cpp`](../../tests/test_day_93.cpp) (7 tests)

## Scenario
The *chat service behind a help-desk tool*: agents connect, pick a nickname, join rooms such as #billing, and messages are broadcast to everyone in the room. The core knows nothing about sockets – it receives bytes and connection events and emits text through an outbox interface – so the same code can sit behind TCP, WebSockets or a unit test.

## Syllabus deliverables
> A line protocol, per-connection sessions, broadcasting, a transport-independent server core

| Deliverable | Implemented in |
|---|---|
| ✅ framing a byte stream into bounded lines | `LineFramer::push` |
| ✅ the outbox the core writes to instead of sockets | `Outbox` |
| ✅ per-connection session state | `Session` |
| ✅ parsing and dispatching protocol commands | `ChatServer::on_line` |
| ✅ broadcasting to a room | `ChatServer::broadcast` |

## Key learnings
- TCP delivers a byte stream, so a line protocol needs a framer that reassembles lines and limits their length.
- Keeping the core free of sockets – bytes and events in, text out through an Outbox – makes it testable and reusable behind any transport.
- Each connection has a session with its own state (nickname, rooms, framer).
- Broadcasting means iterating sessions in a room and skipping the sender; disconnects must be announced and state dropped.

## Pitfalls I hit (and how I fixed them)
- A client that never sent a newline made the server buffer forever; lines over the limit are now discarded with a 413 reply.
- Lines after QUIT in the same packet were still processed for a session that no longer existed.
- A rename was not announced to the rooms, so other users saw messages from an unknown name.

## Run it
```bash
./procpp.sh 93                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_93_network_service       # the interactive demo
ctest --test-dir build -R test_day_93 --output-on-failure
```

## Next step
- Day 94 builds a validation library for untrusted input.
