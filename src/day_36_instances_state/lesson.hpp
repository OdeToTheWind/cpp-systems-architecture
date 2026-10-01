/**
 * @file
 * Day 36 – Instances and State.
 *
 * Scenario: a *parcel-locker network*. Every parcel object tracks its own state as it moves
 * through a lifecycle – registered, in transit, waiting in a locker, collected – or ends up
 * returned to the sender. Illegal jumps (collecting a parcel that never arrived) are refused,
 * and each parcel keeps its own history.
 *
 * Deliverables (syllabus):
 * - Per-object state
 * - State machines with enum class
 * - Transition validation
 * - Lifecycle history
 */
#pragma once

#include <algorithm>
#include <array>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day36 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"the states of the lifecycle as an enum class", "State"},
    {"the transition table that defines legal moves", "allowed"},
    {"per-object state with validated transitions", "Parcel::advance"},
    {"each object keeps its own history", "Parcel::history"},
    {"state-dependent behaviour", "Parcel::collect"},
    {"many independent instances managed together", "LockerNetwork"},
};

enum class State { registered, in_transit, in_locker, collected, returned };

inline std::string_view to_string(State state) {
    switch (state) {
        case State::registered:
            return "registered";
        case State::in_transit:
            return "in transit";
        case State::in_locker:
            return "in locker";
        case State::collected:
            return "collected";
        case State::returned:
            return "returned";
    }
    return "?";
}

/// The whole state machine in one table: from -> every state it may move to.
inline bool allowed(State from, State to) {
    static const std::array<std::pair<State, State>, 6> moves{{
        {State::registered, State::in_transit},
        {State::in_transit, State::in_locker},
        {State::in_transit, State::returned},  // address problem
        {State::in_locker, State::collected},
        {State::in_locker, State::returned},   // not collected in time
        {State::registered, State::returned},  // cancelled before pick-up
    }};
    return std::find(moves.begin(), moves.end(), std::pair{from, to}) != moves.end();
}

class Parcel {
public:
    Parcel(std::string id, std::string pickup_code) : id_(std::move(id)), pickup_code_(std::move(pickup_code)) {
        history_.emplace_back(0, State::registered);
    }

    /// Move to @p next at time @p day; throws std::logic_error for an illegal transition.
    void advance(State next, int day) {
        if (!allowed(state_, next)) {
            throw std::logic_error("parcel " + id_ + ": cannot go from " + std::string(to_string(state_)) + " to " +
                                   std::string(to_string(next)));
        }
        if (day < history_.back().first) {
            throw std::logic_error("time cannot run backwards");
        }
        state_ = next;
        history_.emplace_back(day, next);
    }

    /// Only a parcel waiting in a locker can be collected, and only with its own code.
    bool collect(std::string_view code, int day) {
        if (state_ != State::in_locker || code != pickup_code_) {
            return false;
        }
        advance(State::collected, day);
        return true;
    }

    /// A parcel left in the locker longer than @p max_days goes back to the sender.
    bool expire_if_uncollected(int today, int max_days = 3) {
        if (state_ == State::in_locker && today - history_.back().first > max_days) {
            advance(State::returned, today);
            return true;
        }
        return false;
    }

    const std::string& id() const { return id_; }
    State state() const { return state_; }
    bool finished() const { return state_ == State::collected || state_ == State::returned; }
    const std::vector<std::pair<int, State>>& history() const { return history_; }

private:
    std::string id_;
    std::string pickup_code_;
    State state_{State::registered};
    std::vector<std::pair<int, State>> history_;  // (day, state) – this parcel's own record
};

/// Many parcels, each with independent state.
class LockerNetwork {
public:
    Parcel& add(const std::string& id, const std::string& code) {
        auto [it, inserted] = parcels_.try_emplace(id, id, code);
        if (!inserted) {
            throw std::invalid_argument("duplicate parcel id " + id);
        }
        return it->second;
    }
    Parcel& at(const std::string& id) {
        const auto it = parcels_.find(id);
        if (it == parcels_.end()) {
            throw std::out_of_range("no parcel " + id);
        }
        return it->second;
    }
    /// Run the daily expiry check over every parcel; returns the ids sent back.
    std::vector<std::string> nightly_expiry(int today) {
        std::vector<std::string> returned;
        for (auto& [id, parcel] : parcels_) {
            if (parcel.expire_if_uncollected(today)) {
                returned.push_back(id);
            }
        }
        return returned;
    }
    std::map<State, int> census() const {
        std::map<State, int> counts;
        for (const auto& [id, parcel] : parcels_) {
            ++counts[parcel.state()];
        }
        return counts;
    }

private:
    std::map<std::string, Parcel> parcels_;
};

/// The interactive demo: add/ship/deliver/collect/night commands with a day number.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 36 – Instances and State\n"
           "add <id> <code> | ship <id> <day> | deliver <id> <day> | collect <id> <code> <day> | night <day>\n";
    LockerNetwork network;
    while (auto line = prompt_line(in, out, "> ")) {
        std::istringstream words(*line);
        std::string command;
        std::string id;
        if (!(words >> command)) {
            break;
        }
        try {
            if (command == "night") {
                int day = 0;
                words >> day;
                for (const auto& returned : network.nightly_expiry(day)) {
                    out << "  " << returned << " returned to sender\n";
                }
                continue;
            }
            words >> id;
            if (command == "add") {
                std::string code;
                words >> code;
                network.add(id, code);
            } else if (command == "ship" || command == "deliver") {
                int day = 0;
                words >> day;
                network.at(id).advance(command == "ship" ? State::in_transit : State::in_locker, day);
            } else if (command == "collect") {
                std::string code;
                int day = 0;
                words >> code >> day;
                out << (network.at(id).collect(code, day) ? "  collected\n" : "  wrong code or not in a locker\n");
            } else {
                out << "  unknown command\n";
                continue;
            }
            out << "  " << id << " is " << to_string(network.at(id).state()) << '\n';
        } catch (const std::exception& error) {
            out << "  " << error.what() << '\n';
        }
    }
    for (const auto& [state, count] : network.census()) {
        out << to_string(state) << ": " << count << '\n';
    }
    return 0;
}

}  // namespace cppm::day36
