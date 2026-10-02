/**
 * @file
 * Day 99 – Capstone: Observability Toolkit.
 *
 * Scenario: a *checkout service that fails once in a few thousand orders*, only in production.
 * A debugger cannot be attached there, so the service carries its own instruments: spans that
 * time each step of a request, a flight recorder of recent calls captured with
 * std::source_location, watchpoints that log every change to a suspicious variable, and a crash
 * reporter that turns an exception (with its nested causes) into a report with all of that context.
 *
 * Deliverables (syllabus):
 * - Crash reports
 * - Spans
 * - Call tracing
 * - Watchpoints
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <functional>
#include <istream>
#include <optional>
#include <ostream>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day99 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a flight recorder of recent events", "FlightRecorder"},
    {"recording the caller's file and line automatically", "trace_call"},
    {"RAII spans that build a trace tree", "Span"},
    {"a variable that logs every change", "Watched"},
    {"crash reports with nested causes and context", "crash_report"},
};

inline std::string where(const std::source_location& loc) {
    std::string file = loc.file_name();
    const auto slash = file.find_last_of("/\\");
    if (slash != std::string::npos) file = file.substr(slash + 1);
    return file + ":" + std::to_string(loc.line());
}

/// Keeps only the last N events – cheap enough to leave on in production.
class FlightRecorder {
  public:
    explicit FlightRecorder(std::size_t capacity) : capacity_(capacity) {}
    void record(std::string event) {
        if (events_.size() == capacity_) events_.pop_front();
        events_.push_back(std::move(event));
        ++total_;
    }
    const std::deque<std::string>& events() const { return events_; }
    std::size_t total() const { return total_; }

  private:
    std::size_t capacity_;
    std::deque<std::string> events_;
    std::size_t total_ = 0;
};

/// Record "file:line note". The default argument is evaluated at the *call site*, so callers
/// write `trace_call(rec, "charging card")` and the location is filled in for them.
inline void trace_call(FlightRecorder& recorder, const std::string& note,
                       std::source_location loc = std::source_location::current()) {
    recorder.record(where(loc) + " " + note);
}

using Millis = std::int64_t;

struct SpanRecord {
    int id;
    int parent;  // 0 for a root span
    std::string name;
    Millis start;
    Millis end = -1;  // -1 while open
    std::vector<std::pair<std::string, std::string>> attributes;
    std::string status = "ok";
};

/// Collects spans; the currently open spans form a stack, so new spans get the right parent.
class Tracer {
  public:
    explicit Tracer(std::function<Millis()> clock) : clock_(std::move(clock)) {}
    int open(const std::string& name) {
        const int id = static_cast<int>(spans_.size()) + 1;
        spans_.push_back({id, stack_.empty() ? 0 : stack_.back(), name, clock_(), -1, {}, "ok"});
        stack_.push_back(id);
        return id;
    }
    void close(int id, const std::string& status) {
        span(id).end = clock_();
        span(id).status = status;
        if (!stack_.empty() && stack_.back() == id) stack_.pop_back();
    }
    SpanRecord& span(int id) { return spans_.at(static_cast<std::size_t>(id - 1)); }
    const std::vector<SpanRecord>& spans() const { return spans_; }
    /// Names of the spans still open – exactly where a crash happened.
    std::vector<std::string> open_stack() const {
        std::vector<std::string> names;
        for (const int id : stack_) names.push_back(spans_.at(static_cast<std::size_t>(id - 1)).name);
        return names;
    }
    /// An indented timeline: "checkout 0-130ms ok", "  reserve 0-20ms ok", …
    std::string timeline() const {
        std::string out;
        for (const auto& s : spans_) {
            int depth = 0;
            for (int p = s.parent; p != 0; p = spans_.at(static_cast<std::size_t>(p - 1)).parent) ++depth;
            out += std::string(static_cast<std::size_t>(depth) * 2, ' ') + s.name + " " + std::to_string(s.start) +
                   "-" + (s.end < 0 ? std::string("?") : std::to_string(s.end)) + "ms " + s.status;
            for (const auto& [k, v] : s.attributes) out += " " + k + "=" + v;
            out += "\n";
        }
        return out;
    }

  private:
    std::function<Millis()> clock_;
    std::vector<SpanRecord> spans_;
    std::vector<int> stack_;
};

/// RAII: the span closes when the scope ends – with status "error" if it ends by an exception.
class Span {
  public:
    Span(Tracer& tracer, const std::string& name)
        : tracer_(tracer), id_(tracer.open(name)), exceptions_(std::uncaught_exceptions()) {}
    Span(const Span&) = delete;
    Span& operator=(const Span&) = delete;
    ~Span() { tracer_.close(id_, std::uncaught_exceptions() > exceptions_ ? "error" : "ok"); }
    void attribute(const std::string& key, const std::string& value) {
        tracer_.span(id_).attributes.emplace_back(key, value);
    }

  private:
    Tracer& tracer_;
    int id_;
    int exceptions_;
};

/// A software watchpoint: every assignment is logged with old value, new value and the caller's
/// location, and an optional condition can trigger a callback ("break when stock goes negative").
template <typename T>
class Watched {
  public:
    Watched(std::string name, T value, FlightRecorder& recorder)
        : name_(std::move(name)), value_(std::move(value)), recorder_(recorder) {}
    void set(T value, std::source_location loc = std::source_location::current()) {
        std::ostringstream event;
        event << where(loc) << " watch " << name_ << ": " << value_ << " -> " << value;
        recorder_.record(event.str());
        value_ = std::move(value);
        if (condition_ && condition_(value_)) on_trigger_(event.str());
    }
    const T& get() const { return value_; }
    void break_when(std::function<bool(const T&)> condition, std::function<void(const std::string&)> action) {
        condition_ = std::move(condition);
        on_trigger_ = std::move(action);
    }

  private:
    std::string name_;
    T value_;
    FlightRecorder& recorder_;
    std::function<bool(const T&)> condition_;
    std::function<void(const std::string&)> on_trigger_;
};

/// "what" of an exception and of every exception nested inside it (std::throw_with_nested).
inline std::vector<std::string> exception_chain(const std::exception_ptr& error) {
    std::vector<std::string> chain;
    std::exception_ptr current = error;
    while (current) {
        try {
            std::rethrow_exception(current);
        } catch (const std::exception& e) {
            chain.push_back(e.what());
            try {
                std::rethrow_if_nested(e);
                current = nullptr;
            } catch (...) {
                current = std::current_exception();
            }
        } catch (...) {
            chain.push_back("unknown exception");
            current = nullptr;
        }
    }
    return chain;
}

/// Everything an engineer needs to start debugging, in one block of text.
inline std::string crash_report(const std::exception_ptr& error, const Tracer& tracer, const FlightRecorder& recorder) {
    std::string out = "=== crash report ===\n";
    const auto chain = exception_chain(error);
    for (std::size_t k = 0; k < chain.size(); ++k)
        out += (k == 0 ? "error: " : std::string(k * 2, ' ') + "caused by: ") + chain[k] + "\n";
    out += "open spans:";
    for (const auto& name : tracer.open_stack()) out += " " + name;
    out +=
        "\nlast " + std::to_string(recorder.events().size()) + " of " + std::to_string(recorder.total()) + " events:\n";
    for (const auto& e : recorder.events()) out += "  " + e + "\n";
    return out + "trace:\n" + tracer.timeline();
}

/// The instrumented checkout. Fails when the stock is oversold – the bug being hunted.
struct Checkout {
    Millis now = 0;
    FlightRecorder recorder{8};
    Tracer tracer{[this] { return now; }};
    Watched<int> stock{"stock[sku-42]", 3, recorder};
    std::vector<std::string> breaks;

    Checkout() {
        stock.break_when([](const int& v) { return v < 0; }, [this](const std::string& e) { breaks.push_back(e); });
    }
    void place_order(int order_id, int quantity) {
        Span request(tracer, "checkout#" + std::to_string(order_id));
        request.attribute("qty", std::to_string(quantity));
        trace_call(recorder, "order " + std::to_string(order_id) + " started");
        {
            Span reserve(tracer, "reserve");
            now += 20;
            stock.set(stock.get() - quantity);  // bug: no check before reserving
        }
        try {
            Span charge(tracer, "charge");
            now += 100;
            trace_call(recorder, "charging card");
            if (stock.get() < 0) throw std::runtime_error("inventory went negative");
        } catch (...) {
            std::throw_with_nested(std::runtime_error("order " + std::to_string(order_id) + " failed"));
        }
        now += 10;
    }
};

/// The interactive demo: "order <id> <qty>" places orders until one crashes, then prints the report.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 99 – Capstone: Observability Toolkit\n";
    Checkout service;
    while (auto line = prompt_line(in, out, "order <id> <qty>> ")) {
        std::istringstream words(*line);
        std::string command;
        int id = 0;
        int qty = 0;
        if (!(words >> command >> id >> qty) || command != "order") break;
        try {
            service.place_order(id, qty);
            out << "  order " << id << " ok, stock " << service.stock.get() << '\n';
        } catch (...) {
            out << crash_report(std::current_exception(), service.tracer, service.recorder);
            for (const auto& b : service.breaks) out << "watchpoint hit: " << b << '\n';
            break;
        }
    }
    return 0;
}

}  // namespace cppm::day99
