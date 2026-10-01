/**
 * @file
 * Day 16 – Flowchart Programming.
 *
 * Scenario: an *airport check-in kiosk*. Each rule – the baggage fee, the boarding check
 * and the queue the kiosk works through – is first drawn as a flowchart (kept in the code as
 * Mermaid text) and then translated shape by shape into structured C++.
 *
 * Deliverables (syllabus):
 * - Translating decisions (diamonds) into if / else
 * - Translating processes (rectangles) into statements
 * - Translating loops (arrows back up) into while / for
 */
#pragma once

#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day16 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"flowcharts kept next to the code as Mermaid text", "flowchart"},
    {"decision diamonds become an if / else chain", "baggage_fee_cents"},
    {"early exits for terminal states", "boarding_check"},
    {"a loop arrow becomes a while loop over a queue", "process_queue"},
};

/// The Mermaid source of each flowchart; paste it into any Mermaid viewer to see the diagram.
inline std::string_view flowchart(std::string_view name) {
    if (name == "baggage") {
        return "flowchart TD\n"
               "  A([start]) --> B{weight > 32 kg?}\n"
               "  B -- yes --> R[[refuse: must ship as cargo]]\n"
               "  B -- no --> C{business class?}\n"
               "  C -- yes --> F[fee = 0]\n"
               "  C -- no --> D{weight > 23 kg?}\n"
               "  D -- yes --> G[fee = 75.00]\n"
               "  D -- no --> H[fee = 30.00]\n"
               "  G --> I{frequent flyer?}\n"
               "  H --> I\n"
               "  I -- yes --> J[fee = fee / 2]\n"
               "  I -- no --> K([end])\n"
               "  J --> K\n"
               "  F --> K\n";
    }
    if (name == "boarding") {
        return "flowchart TD\n"
               "  A([start]) --> B{passport valid?}\n"
               "  B -- no --> X[[deny: passport]]\n"
               "  B -- yes --> C{visa required?}\n"
               "  C -- yes --> D{has visa?}\n"
               "  D -- no --> Y[[deny: visa]]\n"
               "  C -- no --> E{minutes to departure >= 45?}\n"
               "  D -- yes --> E\n"
               "  E -- no --> Z[[send to desk]]\n"
               "  E -- yes --> OK[[issue boarding pass]]\n";
    }
    if (name == "queue") {
        return "flowchart TD\n"
               "  A([start]) --> B{queue empty?}\n"
               "  B -- yes --> E([end])\n"
               "  B -- no --> C[take next passenger]\n"
               "  C --> D[run boarding check and count result]\n"
               "  D --> B\n";
    }
    return "";
}

/// Number of decision diamonds `{...}` in a flowchart – each must become one condition in C++.
inline int count_decisions(std::string_view chart) {
    int decisions = 0;
    for (std::size_t at = chart.find('{'); at != std::string_view::npos; at = chart.find('{', at + 1)) {
        ++decisions;
    }
    return decisions;
}

enum class Cabin { economy, business };

/// Baggage fee in cents, or -1 when the bag must be refused (see flowchart("baggage")).
inline int baggage_fee_cents(double weight_kg, Cabin cabin, bool frequent_flyer) {
    if (weight_kg > 32) {  // diamond B
        return -1;
    }
    if (cabin == Cabin::business) {  // diamond C
        return 0;
    }
    int fee = weight_kg > 23 ? 7'500 : 3'000;  // diamond D and rectangles G/H
    if (frequent_flyer) {                       // diamond I
        fee /= 2;
    }
    return fee;
}

enum class Boarding { boarding_pass, deny_passport, deny_visa, see_desk };

struct Passenger {
    std::string name;
    bool passport_valid{true};
    bool visa_required{false};
    bool has_visa{false};
    int minutes_to_departure{120};
};

/// Every terminal shape of flowchart("boarding") is a `return`.
inline Boarding boarding_check(const Passenger& p) {
    if (!p.passport_valid) {
        return Boarding::deny_passport;
    }
    if (p.visa_required && !p.has_visa) {
        return Boarding::deny_visa;
    }
    if (p.minutes_to_departure < 45) {
        return Boarding::see_desk;
    }
    return Boarding::boarding_pass;
}

/// How the kiosk's queue ended.
struct QueueSummary {
    int boarding_passes{0};
    int denied{0};
    int sent_to_desk{0};
    std::vector<std::string> desk_names;
};

/// flowchart("queue"): the arrow from D back to B is the while loop.
inline QueueSummary process_queue(std::vector<Passenger> queue) {
    QueueSummary summary;
    std::size_t next = 0;
    while (next < queue.size()) {          // diamond B: queue empty?
        const Passenger& p = queue[next];  // rectangle C
        ++next;
        switch (boarding_check(p)) {  // rectangle D
            case Boarding::boarding_pass:
                ++summary.boarding_passes;
                break;
            case Boarding::deny_passport:
            case Boarding::deny_visa:
                ++summary.denied;
                break;
            case Boarding::see_desk:
                ++summary.sent_to_desk;
                summary.desk_names.push_back(p.name);
                break;
        }
    }
    return summary;
}

/// The interactive demo: print a flowchart, then price bags as "weight class(e/b) frequent(0/1)".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 16 – Flowchart Programming\n" << flowchart("baggage");
    out << "(" << count_decisions(flowchart("baggage")) << " decisions -> " << count_decisions(flowchart("baggage"))
        << " conditions in baggage_fee_cents)\n";
    while (auto line = prompt_line(in, out, "weight class(e/b) frequent(0/1)> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream values(*line);
        double weight{};
        char cabin{};
        bool frequent{};
        if (!(values >> weight >> cabin >> frequent) || (cabin != 'e' && cabin != 'b') || weight < 0) {
            out << "  e.g. 25 e 1\n";
            continue;
        }
        const int fee = baggage_fee_cents(weight, cabin == 'b' ? Cabin::business : Cabin::economy, frequent);
        if (fee < 0) {
            out << "  refused: ship it as cargo\n";
        } else {
            out << "  fee " << fee / 100 << '.' << (fee % 100 < 10 ? "0" : "") << fee % 100 << '\n';
        }
    }
    const auto summary = process_queue({{"Ada"}, {"Bo", false}, {"Cy", true, true, false}, {"Di", true, false, false, 30}});
    out << "Queue: " << summary.boarding_passes << " boarded, " << summary.denied << " denied, "
        << summary.sent_to_desk << " sent to the desk\n";
    return 0;
}

}  // namespace cppm::day16
