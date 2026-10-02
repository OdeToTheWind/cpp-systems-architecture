// Tests for Day 99 – Capstone: Observability Toolkit. Locations are checked by file and line only:
// function names from std::source_location are spelled differently by each compiler.
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_99_observability/lesson.hpp"

using namespace cppm::day99;

TEST_CASE("the flight recorder keeps only the latest events") {
    FlightRecorder rec(3);
    for (int k = 1; k <= 5; ++k) rec.record("e" + std::to_string(k));
    CHECK(std::vector<std::string>(rec.events().begin(), rec.events().end()) == std::vector<std::string>{"e3", "e4", "e5"});
    CHECK_EQ(rec.total(), 5u);
}

TEST_CASE("trace_call records the caller's file and line") {
    FlightRecorder rec(4);
    const int line = __LINE__ + 1;
    trace_call(rec, "hello");
    CHECK_EQ(rec.events().back(), "test_day_99.cpp:" + std::to_string(line) + " hello");
}

TEST_CASE("spans nest, time and close themselves") {
    Millis now = 0;
    Tracer tracer([&now] { return now; });
    {
        Span root(tracer, "request");
        root.attribute("user", "ana");
        {
            Span db(tracer, "db");
            now += 15;
        }
        now += 5;
    }
    CHECK_EQ(tracer.timeline(), "request 0-20ms ok user=ana\n  db 0-15ms ok\n");
    CHECK(tracer.open_stack().empty());
}

TEST_CASE("a span left by an exception is marked as an error") {
    Millis now = 0;
    Tracer tracer([&now] { return now; });
    try {
        Span s(tracer, "risky");
        throw std::runtime_error("x");
    } catch (const std::exception&) {
    }
    CHECK_EQ(tracer.spans()[0].status, "error");
    try {  // a span created while another exception is in flight is not blamed for it
        throw 1;
    } catch (int) {
        Span inside(tracer, "handler");
    }
    CHECK_EQ(tracer.spans()[1].status, "ok");
}

TEST_CASE("watchpoints log every change and trigger on a condition") {
    FlightRecorder rec(10);
    Watched<int> balance("balance", 10, rec);
    std::vector<std::string> hits;
    balance.break_when([](const int& v) { return v < 0; }, [&hits](const std::string& e) { hits.push_back(e); });
    balance.set(4);
    const int line = __LINE__ + 1;
    balance.set(-2);
    CHECK_EQ(rec.events().front().substr(rec.events().front().find(" watch")), " watch balance: 10 -> 4");
    CHECK_EQ(hits.size(), 1u);
    CHECK_EQ(hits[0], "test_day_99.cpp:" + std::to_string(line) + " watch balance: 4 -> -2");
}

TEST_CASE("nested exceptions are unwound into a causal chain") {
    std::exception_ptr error;
    try {
        try {
            throw std::out_of_range("index 7");
        } catch (...) {
            std::throw_with_nested(std::runtime_error("loading cart"));
        }
    } catch (...) {
        error = std::current_exception();
    }
    CHECK(exception_chain(error) == std::vector<std::string>{"loading cart", "index 7"});
    CHECK(exception_chain(std::make_exception_ptr(42)) == std::vector<std::string>{"unknown exception"});
}

TEST_CASE("the checkout crash report has everything needed to debug it") {
    Checkout service;
    service.place_order(1, 2);
    std::string report;
    try {
        service.place_order(2, 2);
    } catch (...) {
        report = crash_report(std::current_exception(), service.tracer, service.recorder);
    }
    CHECK(report.find("error: order 2 failed\n  caused by: inventory went negative\n") != std::string::npos);
    CHECK(report.find("open spans:\n") != std::string::npos);  // all spans closed during unwinding
    CHECK(report.find("watch stock[sku-42]: 1 -> -1") != std::string::npos);
    CHECK(report.find("checkout#2 130-250ms error qty=2\n  reserve 130-150ms ok\n  charge 150-250ms error\n") != std::string::npos);
    CHECK_EQ(service.breaks.size(), 1u);
    std::istringstream in("order 1 1\norder 2 5\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("order 1 ok, stock 2") != std::string::npos);
    CHECK(out.str().find("watchpoint hit: ") != std::string::npos);
}
