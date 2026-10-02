/**
 * @file
 * Day 93 – Capstone: Network Service Core.
 *
 * Scenario: the *chat service behind a help-desk tool*: agents connect, pick a nickname, join
 * rooms such as #billing, and messages are broadcast to everyone in the room. The core knows
 * nothing about sockets – it receives bytes and connection events and emits text through an
 * outbox interface – so the same code can sit behind TCP, WebSockets or a unit test.
 *
 * Deliverables (syllabus):
 * - A line-based protocol
 * - Sessions
 * - Broadcasting
 * - Transport independence
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day93 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"framing a byte stream into bounded lines", "LineFramer::push"},
    {"the outbox the core writes to instead of sockets", "Outbox"},
    {"per-connection session state", "Session"},
    {"parsing and dispatching protocol commands", "ChatServer::on_line"},
    {"broadcasting to a room", "ChatServer::broadcast"},
};

/// TCP delivers bytes, not messages. The framer collects bytes until "\n" and refuses lines
/// longer than a limit, so one client cannot make the server buffer unbounded data.
class LineFramer {
  public:
    explicit LineFramer(std::size_t max_line = 512) : max_line_(max_line) {}
    /// Returns complete lines; an over-long line becomes std::nullopt and is skipped up to its end.
    std::vector<std::optional<std::string>> push(std::string_view bytes) {
        std::vector<std::optional<std::string>> lines;
        for (const char c : bytes) {
            if (c == '\n') {
                if (overflow_)
                    lines.emplace_back(std::nullopt);
                else
                    lines.emplace_back(buffer_);
                buffer_.clear();
                overflow_ = false;
            } else if (c != '\r' && !overflow_) {
                buffer_ += c;
                if (buffer_.size() > max_line_) {
                    overflow_ = true;
                    buffer_.clear();
                }
            }
        }
        return lines;
    }

  private:
    std::size_t max_line_;
    std::string buffer_;
    bool overflow_ = false;
};

using ConnId = int;

/// Where the core's output goes. A TCP server would write to sockets; tests record it.
class Outbox {
  public:
    virtual ~Outbox() = default;
    virtual void send(ConnId to, const std::string& line) = 0;
    virtual void close(ConnId conn) = 0;
};

/// Everything the server remembers about one connection.
struct Session {
    std::string nick;  // empty until NICK
    std::set<std::string> rooms;
    LineFramer framer;
};

/// Protocol (one command per line, replies start with a status code):
///   NICK <name>          set a nickname (letters, digits, _ ; 2-16 chars; unique)
///   JOIN <#room>         join a room (others in it see "* nick joined")
///   PART <#room>         leave a room
///   MSG <#room> <text>   broadcast to the room
///   WHO <#room>          list members
///   QUIT                 disconnect
class ChatServer {
  public:
    explicit ChatServer(Outbox& outbox) : outbox_(outbox) {}

    void on_connect(ConnId id) {
        sessions_[id];
        outbox_.send(id, "100 welcome, please NICK <name>");
    }
    /// Raw bytes from the transport; may contain several lines or part of one.
    void on_bytes(ConnId id, std::string_view bytes) {
        const auto it = sessions_.find(id);
        if (it == sessions_.end()) return;
        for (const auto& line : it->second.framer.push(bytes)) {
            if (!sessions_.contains(id)) return;  // a previous line was QUIT
            if (line)
                on_line(id, *line);
            else
                outbox_.send(id, "413 line too long");
        }
    }
    void on_line(ConnId id, const std::string& line) {
        Session& s = sessions_.at(id);
        std::istringstream words(line);
        std::string command;
        words >> command;
        for (auto& c : command) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        std::string arg;
        words >> arg;
        std::string rest;
        std::getline(words >> std::ws, rest);
        if (command.empty()) return;
        if (command == "QUIT") {
            outbox_.send(id, "221 bye");
            on_disconnect(id);
            outbox_.close(id);
            return;
        }
        if (command == "NICK") return nick(id, s, arg);
        if (s.nick.empty()) return outbox_.send(id, "401 set a nickname first");
        if (command == "JOIN") return join(id, s, arg);
        if (command == "PART") {
            if (!s.rooms.erase(arg)) return outbox_.send(id, "404 not in " + arg);
            broadcast(arg, "* " + s.nick + " left " + arg);
            return outbox_.send(id, "200 left " + arg);
        }
        if (command == "MSG") {
            if (!s.rooms.contains(arg)) return outbox_.send(id, "404 not in " + arg);
            if (rest.empty()) return outbox_.send(id, "400 empty message");
            broadcast(arg, "<" + s.nick + "@" + arg + "> " + rest, id);
            return outbox_.send(id, "200 sent");
        }
        if (command == "WHO") {
            std::string names;
            for (const auto& [other, session] : sessions_) {
                if (session.rooms.contains(arg)) names += (names.empty() ? "" : " ") + session.nick;
            }
            return outbox_.send(id, "200 " + arg + ": " + (names.empty() ? "(empty)" : names));
        }
        outbox_.send(id, "400 unknown command " + command);
    }
    /// The transport reports a closed connection (or QUIT): rooms are told, state is dropped.
    void on_disconnect(ConnId id) {
        const auto it = sessions_.find(id);
        if (it == sessions_.end()) return;
        const Session gone = it->second;
        sessions_.erase(it);
        for (const auto& room : gone.rooms) broadcast(room, "* " + gone.nick + " disconnected");
    }
    /// Send @p line to every session in @p room, optionally except one (the sender).
    void broadcast(const std::string& room, const std::string& line, std::optional<ConnId> except = std::nullopt) {
        for (const auto& [id, session] : sessions_) {
            if (session.rooms.contains(room) && id != except) outbox_.send(id, line);
        }
    }
    std::size_t connections() const { return sessions_.size(); }

  private:
    void nick(ConnId id, Session& s, const std::string& name) {
        const bool valid =
            name.size() >= 2 && name.size() <= 16 &&
            std::all_of(name.begin(), name.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; });
        if (!valid) return outbox_.send(id, "400 nicknames are 2-16 letters, digits or _");
        for (const auto& [other, session] : sessions_) {
            if (other != id && session.nick == name) return outbox_.send(id, "409 nickname in use");
        }
        const std::string old = s.nick;
        s.nick = name;
        for (const auto& room : s.rooms) broadcast(room, "* " + old + " is now " + name, id);
        outbox_.send(id, "200 hello " + name);
    }
    void join(ConnId id, Session& s, const std::string& room) {
        if (room.size() < 2 || room[0] != '#') return outbox_.send(id, "400 rooms start with #");
        if (!s.rooms.insert(room).second) return outbox_.send(id, "409 already in " + room);
        broadcast(room, "* " + s.nick + " joined " + room, id);
        outbox_.send(id, "200 joined " + room);
    }

    Outbox& outbox_;
    std::map<ConnId, Session> sessions_;
};

/// An outbox that records everything – what a test (or the demo) uses instead of sockets.
class RecordingOutbox : public Outbox {
  public:
    void send(ConnId to, const std::string& line) override { lines.push_back({to, line}); }
    void close(ConnId conn) override { closed.push_back(conn); }
    std::vector<std::string> to(ConnId id) const {
        std::vector<std::string> out;
        for (const auto& [conn, line] : lines) {
            if (conn == id) out.push_back(line);
        }
        return out;
    }
    std::vector<std::pair<ConnId, std::string>> lines;
    std::vector<ConnId> closed;
};

/// The interactive demo plays several clients: "<conn> connect", "<conn> <protocol line>", "<conn> drop".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 93 – Capstone: Network Service Core\n";
    RecordingOutbox outbox;
    ChatServer server(outbox);
    std::size_t shown = 0;
    while (auto line = prompt_line(in, out, "<conn> connect|drop|<command>> ")) {
        std::istringstream words(*line);
        ConnId conn = 0;
        if (!(words >> conn)) break;
        std::string rest;
        std::getline(words >> std::ws, rest);
        if (rest == "connect")
            server.on_connect(conn);
        else if (rest == "drop")
            server.on_disconnect(conn);
        else
            server.on_bytes(conn, rest + "\r\n");
        for (; shown < outbox.lines.size(); ++shown)
            out << "  -> " << outbox.lines[shown].first << ": " << outbox.lines[shown].second << '\n';
    }
    out << server.connections() << " connection(s) open\n";
    return 0;
}

}  // namespace cppm::day93
