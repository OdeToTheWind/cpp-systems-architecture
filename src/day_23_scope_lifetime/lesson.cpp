// Definitions for lesson.hpp – this file is where linkage and lifetime actually matter.
#include "lesson.hpp"

#include <iomanip>
#include <memory>
#include <sstream>

namespace cppm::day23 {
namespace {  // internal linkage: these names are invisible to every other translation unit

int issued = 0;  // file-scope state, but private to this file instead of a true global

std::string two_digits(int number) {
    std::ostringstream out;
    out << std::setw(2) << std::setfill('0') << number;
    return out.str();
}

/// Logs its own birth and death into a shared log.
class Tracked {
public:
    Tracked(std::vector<std::string>& log, std::string name) : log_(&log), name_(std::move(name)) {
        log_->push_back("construct " + name_);
    }
    ~Tracked() { log_->push_back("destroy " + name_); }
    Tracked(const Tracked&) = delete;
    Tracked& operator=(const Tracked&) = delete;

private:
    std::vector<std::string>* log_;
    std::string name_;
};

void touch_static(std::vector<std::string>& log) {
    static Tracked first_use_only(log, "static (lives until exit)");  // constructed on the first call only
    log.emplace_back("touch_static called");
}

}  // namespace

int next_ticket() {
    static int counter = shop::first_ticket - 1;  // initialised exactly once, thread-safely
    counter = counter >= shop::last_ticket ? shop::first_ticket : counter + 1;
    ++issued;
    return counter;
}

int tickets_issued() { return issued; }

std::string format_ticket(int number) { return std::string(shop::counter_name) + " #" + two_digits(number); }

std::pair<int, int> shadowing_demo() {
    const int waiting = 3;  // outer
    int inner_value = 0;
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"  // -Wshadow exists precisely to flag the next line
#endif
    {
        const int waiting = 10;  // shadows the outer `waiting` until the closing brace
        inner_value = waiting;
    }
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    return {waiting, inner_value};
}

std::vector<std::string> lifetime_demo() {
    static std::vector<std::string> log;  // must outlive the static Tracked object that writes to it
    log.clear();
    {
        Tracked automatic(log, "automatic");
        auto dynamic = std::make_unique<Tracked>(log, "dynamic");
        touch_static(log);
        touch_static(log);
        dynamic.reset();  // a heap object dies when its owner says so
        log.emplace_back("end of block");
    }  // `automatic` dies here
    return log;
}

int run(std::istream& in, std::ostream& out) {
    out << "Day 23 – Scope, Lifetime & Global Variables\n";
    const auto [outer, inner] = shadowing_demo();
    out << "  shadowing: inner block saw " << inner << ", outer still " << outer << '\n';
    out << shop::counter_name << " counter open – Enter for a ticket, q to close\n";
    while (auto line = prompt_line(in, out, "")) {
        if (*line == "q") {
            break;
        }
        out << "  " << format_ticket(next_ticket()) << '\n';
    }
    out << tickets_issued() << " ticket(s) issued since start-up\n";
    return 0;
}

}  // namespace cppm::day23
