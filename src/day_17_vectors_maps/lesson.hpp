/**
 * @file
 * Day 17 – Vectors and Maps.
 *
 * Scenario: a *community tool library*. A `std::map` keeps the stock of every tool by name,
 * a `std::vector` keeps the waiting list in arrival order, and borrow counts are sorted into
 * a "most popular tools" board.
 *
 * Deliverables (syllabus):
 * - std::vector and std::map insert, lookup, update and erase
 * - Iteration
 * - Choosing the right container
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day17 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"map insert-or-update with operator[]", "Inventory::add"},
    {"map lookup with find() that never inserts", "Inventory::quantity"},
    {"map erase by key", "Inventory::remove"},
    {"iterating a map in key order", "Inventory::report"},
    {"vector push_back, erase and order", "Waitlist"},
    {"copying a map into a vector to sort by value", "most_borrowed"},
    {"the operator[] lookup pitfall", "brackets_insert_missing_keys"},
};

/// Tool stock keyed by name. std::map keeps keys sorted and gives O(log n) lookup.
class Inventory {
public:
    /// Insert a new tool or add to an existing one; operator[] value-initialises missing keys to 0.
    void add(const std::string& tool, int count) {
        if (count <= 0) {
            return;
        }
        stock_[tool] += count;
    }

    /// How many are on the shelf; find() does not create an entry for unknown tools.
    int quantity(const std::string& tool) const {
        const auto it = stock_.find(tool);
        return it == stock_.end() ? 0 : it->second;
    }

    /// Lend one tool; returns false when none is available. Updates the value through the iterator.
    bool lend(const std::string& tool) {
        const auto it = stock_.find(tool);
        if (it == stock_.end() || it->second == 0) {
            return false;
        }
        --it->second;
        ++borrowed_[tool];
        return true;
    }

    void give_back(const std::string& tool) { add(tool, 1); }

    /// Remove a tool entirely; returns true if it existed.
    bool remove(const std::string& tool) { return stock_.erase(tool) == 1; }

    /// "name: count" lines in alphabetical order – map iteration is always sorted by key.
    std::vector<std::string> report() const {
        std::vector<std::string> lines;
        lines.reserve(stock_.size());
        for (const auto& [tool, count] : stock_) {
            lines.push_back(tool + ": " + std::to_string(count));
        }
        return lines;
    }

    const std::map<std::string, int>& borrow_counts() const { return borrowed_; }
    std::size_t distinct_tools() const { return stock_.size(); }

private:
    std::map<std::string, int> stock_;
    std::map<std::string, int> borrowed_;
};

/// First come, first served: a vector keeps insertion order.
class Waitlist {
public:
    /// Joins the end of the list; a person already waiting is not added twice.
    bool join(const std::string& person) {
        if (std::find(people_.begin(), people_.end(), person) != people_.end()) {
            return false;
        }
        people_.push_back(person);
        return true;
    }
    /// Leave from anywhere in the list (C++20 std::erase removes every match).
    bool leave(const std::string& person) { return std::erase(people_, person) > 0; }
    /// Serve the person at the front.
    std::optional<std::string> next() {
        if (people_.empty()) {
            return std::nullopt;
        }
        std::string first = people_.front();
        people_.erase(people_.begin());
        return first;
    }
    const std::vector<std::string>& people() const { return people_; }

private:
    std::vector<std::string> people_;
};

/// The @p n most borrowed tools. A map is ordered by key, so to order by value copy into a vector.
inline std::vector<std::pair<std::string, int>> most_borrowed(const std::map<std::string, int>& counts,
                                                              std::size_t n) {
    std::vector<std::pair<std::string, int>> ranking(counts.begin(), counts.end());
    std::stable_sort(ranking.begin(), ranking.end(),
                     [](const auto& a, const auto& b) { return a.second > b.second; });
    if (ranking.size() > n) {
        ranking.resize(n);
    }
    return ranking;
}

/// Reading a missing key with operator[] silently inserts it – returns the size before and after.
inline std::pair<std::size_t, std::size_t> brackets_insert_missing_keys() {
    std::map<std::string, int> stock{{"drill", 2}};
    const std::size_t before = stock.size();
    const int ladders = stock["ladder"];  // looks like a read, but inserts {"ladder", 0}
    static_cast<void>(ladders);
    return {before, stock.size()};
}

/// The interactive demo: commands add/lend/return/remove/wait/next/list.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 17 – Vectors and Maps\ncommands: add <tool> <n> | lend <tool> | return <tool> | remove <tool>\n"
           "          wait <name> | next | list | top\n";
    Inventory inventory;
    Waitlist waitlist;
    while (auto line = prompt_line(in, out, "> ")) {
        std::istringstream words(*line);
        std::string command;
        std::string name;
        if (!(words >> command)) {
            break;
        }
        words >> name;
        if (command == "add") {
            int count = 0;
            words >> count;
            inventory.add(name, count);
        } else if (command == "lend") {
            out << (inventory.lend(name) ? "  lent " : "  none available: ") << name << '\n';
        } else if (command == "return") {
            inventory.give_back(name);
        } else if (command == "remove") {
            out << (inventory.remove(name) ? "  removed " : "  unknown tool ") << name << '\n';
        } else if (command == "wait") {
            out << (waitlist.join(name) ? "  added to waitlist\n" : "  already waiting\n");
        } else if (command == "next") {
            out << "  next: " << waitlist.next().value_or("(nobody)") << '\n';
        } else if (command == "list") {
            for (const auto& entry : inventory.report()) {
                out << "  " << entry << '\n';
            }
        } else if (command == "top") {
            for (const auto& [tool, count] : most_borrowed(inventory.borrow_counts(), 3)) {
                out << "  " << tool << " x" << count << '\n';
            }
        } else {
            out << "  unknown command\n";
        }
    }
    return 0;
}

}  // namespace cppm::day17
