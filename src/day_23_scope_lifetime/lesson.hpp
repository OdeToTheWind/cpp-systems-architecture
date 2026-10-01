/**
 * @file
 * Day 23 – Scope, Lifetime & Global Variables.
 *
 * Scenario: a *deli-counter ticket dispenser*. Shop-wide settings live in a namespace, the
 * ticket counter is a function-local static that survives between calls, helper code is
 * hidden with internal linkage, and a lifetime log shows exactly when each object is born
 * and destroyed.
 *
 * Deliverables (syllabus):
 * - Block, function, namespace and static scope
 * - Shadowing
 * - Object lifetime
 * - Internal linkage
 */
#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day23 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"namespace-scope constants instead of mutable globals", "shop"},
    {"a function-local static that keeps its value between calls", "next_ticket"},
    {"internal linkage hides helpers in lesson.cpp", "format_ticket"},
    {"shadowing: an inner name hides an outer one", "shadowing_demo"},
    {"automatic, static and dynamic lifetimes side by side", "lifetime_demo"},
};

/// Namespace scope: visible to everyone who includes this header, but grouped and read-only.
namespace shop {
inline constexpr int first_ticket = 1;
inline constexpr int last_ticket = 99;  // the display has two digits
inline constexpr const char* counter_name = "Deli";
}  // namespace shop

/// The next ticket number, 1..99 then back to 1. The counter is a static local: initialised
/// once, on the first call, and it lives until the program ends.
int next_ticket();

/// How many tickets next_ticket() has issued in this run of the program.
int tickets_issued();

/// "Deli #07" – uses a helper that has internal linkage inside lesson.cpp.
std::string format_ticket(int number);

/// Returns {outer value, inner value} for a name declared twice in nested blocks.
std::pair<int, int> shadowing_demo();

/// Records construction and destruction of automatic, static and dynamic objects.
std::vector<std::string> lifetime_demo();

/// The interactive demo: press Enter for a ticket, 'q' to close the counter.
int run(std::istream& in, std::ostream& out);

}  // namespace cppm::day23
