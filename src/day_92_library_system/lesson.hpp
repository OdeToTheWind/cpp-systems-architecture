/**
 * @file
 * Day 92 – Capstone: Multi-module Library with a Full Test Suite.
 *
 * Scenario: the *lending system of a community library*. The code is split into modules with
 * one-way dependencies – model (plain data) <- ports (interfaces plus in-memory doubles) <-
 * service (the rules) – each compiled separately. The test suite exercises each module on its own
 * and the service through fakes and a fixed clock: borrowing, limits, renewals, reservation
 * queues, fines and notifications.
 *
 * Deliverables (syllabus):
 * - Model, repository, notifier and service modules
 * - Fakes
 * - A fixed clock
 * - A full test suite
 */
#pragma once

#include <istream>
#include <ostream>
#include <sstream>
#include <string>

#include "cppm/lesson.hpp"
#include "library/model.hpp"
#include "library/ports.hpp"
#include "library/service.hpp"

namespace cppm::day92 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"dates as day numbers with calendar conversion", "Date::from_ymd"},
    {"the repository port and its in-memory fake", "InMemoryRepository"},
    {"a fixed clock for deterministic due dates", "FixedClock"},
    {"borrowing rules: limits, overdue checks, held copies", "LendingService::borrow"},
    {"returns with fines and reservation notices", "LendingService::return_book"},
};

/// The demo library with a few books and members, all in memory.
struct DemoLibrary {
    InMemoryRepository repo;
    RecordingNotifier notifier;
    FixedClock clock{Date::from_ymd(2026, 6, 12)};
    LendingService service{repo, notifier, clock};
    DemoLibrary() {
        repo.add_book({"978-0141439518", "Pride and Prejudice", 1});
        repo.add_book({"978-0441172719", "Dune", 2});
        repo.add_member({"m1", "Ana", "ana@example.org"});
        repo.add_member({"m2", "Ben", "ben@example.org"});
    }
};

/// The interactive demo: borrow <member> <isbn> | return <loan> | renew <loan> | reserve <member> <isbn> | wait <days>
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 92 – Capstone: Multi-module Library with a Full Test Suite\n";
    DemoLibrary lib;
    std::size_t shown = 0;
    while (auto line = prompt_line(in, out, "borrow|return|renew|reserve|wait> ")) {
        std::istringstream words(*line);
        std::string command;
        if (!(words >> command)) break;
        try {
            if (command == "borrow") {
                std::string member;
                std::string isbn;
                words >> member >> isbn;
                const auto loan = lib.service.borrow(member, isbn);
                out << "  loan #" << loan.id << " due " << loan.due.str() << '\n';
            } else if (command == "return") {
                int id = 0;
                words >> id;
                const auto result = lib.service.return_book(id);
                out << "  returned; fine " << result.fine_cents << " cents\n";
            } else if (command == "renew") {
                int id = 0;
                words >> id;
                out << "  now due " << lib.service.renew(id).due.str() << '\n';
            } else if (command == "reserve") {
                std::string member;
                std::string isbn;
                words >> member >> isbn;
                out << "  position " << lib.service.reserve(member, isbn) << " in the queue\n";
            } else if (command == "wait") {
                int days = 0;
                words >> days;
                lib.clock.advance(days);
                out << "  today is " << lib.clock.today().str() << '\n';
            }
        } catch (const LendingError& error) {
            out << "  refused: " << error.what() << '\n';
        }
        for (; shown < lib.notifier.sent.size(); ++shown)
            out << "  email to " << lib.notifier.sent[shown].to << ": " << lib.notifier.sent[shown].text << '\n';
    }
    return 0;
}

}  // namespace cppm::day92
