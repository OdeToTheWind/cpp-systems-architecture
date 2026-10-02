// Tests for Day 93 – Capstone: Network Service Core. No sockets: bytes go in, recorded lines come out.
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_93_network_service/lesson.hpp"

using namespace cppm::day93;

namespace {
struct Harness {
    RecordingOutbox outbox;
    ChatServer server{outbox};
    void login(ConnId id, const std::string& nick, const std::string& room = "") {
        server.on_connect(id);
        server.on_bytes(id, "NICK " + nick + "\n");
        if (!room.empty()) server.on_bytes(id, "JOIN " + room + "\n");
    }
    std::string last(ConnId id) const { return outbox.to(id).empty() ? "" : outbox.to(id).back(); }
};
}  // namespace

TEST_CASE("the framer reassembles split lines and drops over-long ones") {
    LineFramer framer(8);
    CHECK(framer.push("NI").empty());
    const auto a = framer.push("CK ann\r\nJOIN #x\nPAR");
    CHECK_EQ(a.size(), 2u);
    CHECK_EQ(*a[0], "NICK ann");
    CHECK_EQ(*a[1], "JOIN #x");
    const auto b = framer.push("T\n" + std::string(20, 'A') + "\nok\n");
    CHECK_EQ(*b[0], "PART");
    CHECK(!b[1].has_value());
    CHECK_EQ(*b[2], "ok");
}

TEST_CASE("sessions need a valid, unique nickname first") {
    Harness h;
    h.server.on_connect(1);
    CHECK_EQ(h.last(1), "100 welcome, please NICK <name>");
    h.server.on_bytes(1, "JOIN #billing\n");
    CHECK_EQ(h.last(1), "401 set a nickname first");
    h.server.on_bytes(1, "NICK a\n");
    CHECK_EQ(h.last(1), "400 nicknames are 2-16 letters, digits or _");
    h.server.on_bytes(1, "nick ana\n");  // commands are case-insensitive
    CHECK_EQ(h.last(1), "200 hello ana");
    h.login(2, "ana");
    CHECK_EQ(h.last(2), "409 nickname in use");
}

TEST_CASE("messages are broadcast to the room, not echoed or leaked") {
    Harness h;
    h.login(1, "ana", "#billing");
    h.login(2, "ben", "#billing");
    h.login(3, "cy", "#tech");
    CHECK_EQ(h.last(1), "* ben joined #billing");
    h.server.on_bytes(2, "MSG #billing invoice 42 is paid\n");
    CHECK_EQ(h.last(1), "<ben@#billing> invoice 42 is paid");
    CHECK_EQ(h.last(2), "200 sent");
    CHECK_EQ(h.last(3), "200 joined #tech");  // other rooms see nothing
    h.server.on_bytes(3, "MSG #billing hi\n");
    CHECK_EQ(h.last(3), "404 not in #billing");
}

TEST_CASE("WHO, PART and renames are announced") {
    Harness h;
    h.login(1, "ana", "#ops");
    h.login(2, "ben", "#ops");
    h.server.on_bytes(1, "WHO #ops\n");
    CHECK_EQ(h.last(1), "200 #ops: ana ben");
    h.server.on_bytes(2, "NICK benny\n");
    CHECK_EQ(h.last(1), "* ben is now benny");
    h.server.on_bytes(2, "PART #ops\n");
    CHECK_EQ(h.last(1), "* benny left #ops");
    h.server.on_bytes(1, "WHO #empty\n");
    CHECK_EQ(h.last(1), "200 #empty: (empty)");
}

TEST_CASE("disconnects and QUIT clean up and tell the room") {
    Harness h;
    h.login(1, "ana", "#ops");
    h.login(2, "ben", "#ops");
    h.server.on_bytes(2, "QUIT\nMSG #ops after quit\n");
    CHECK_EQ(h.last(1), "* ben disconnected");
    CHECK(h.outbox.closed == std::vector<ConnId>{2});
    CHECK_EQ(h.server.connections(), 1u);
    h.server.on_disconnect(1);
    h.server.on_disconnect(1);  // twice is harmless
    CHECK_EQ(h.server.connections(), 0u);
}

TEST_CASE("bad input gets error replies, never a crash") {
    Harness h;
    h.login(1, "ana");
    h.server.on_bytes(1, "JOIN billing\n");
    CHECK_EQ(h.last(1), "400 rooms start with #");
    h.server.on_bytes(1, "DANCE\n");
    CHECK_EQ(h.last(1), "400 unknown command DANCE");
    h.server.on_bytes(1, std::string(600, 'x') + "\n");
    CHECK_EQ(h.last(1), "413 line too long");
    h.server.on_bytes(99, "NICK ghost\n");  // unknown connection: ignored
    h.server.on_bytes(1, "\n");
    CHECK_EQ(h.last(1), "413 line too long");  // empty lines are ignored: no new reply
}

TEST_CASE("run simulates several clients") {
    std::istringstream in("1 connect\n2 connect\n1 NICK ana\n2 NICK ben\n1 JOIN #x\n2 JOIN #x\n1 MSG #x hello\n2 drop\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("-> 2: <ana@#x> hello") != std::string::npos);
    CHECK(out.str().find("-> 1: * ben disconnected") != std::string::npos);
    CHECK(out.str().find("1 connection(s) open") != std::string::npos);
}
