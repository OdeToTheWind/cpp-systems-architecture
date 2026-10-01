// The shared runner behind every tests/test_day_XX.cpp executable.
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

#include "cppm/testing.hpp"

namespace cppm::testing {
namespace {

struct Counters {
    int checks = 0;
    int failed_checks = 0;
    bool current_failed = false;
};

Counters& counters() {
    static Counters instance;
    return instance;
}

}  // namespace

std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

void report_check(bool passed, std::string_view expression, std::string_view detail, const char* file,
                  int line) {
    auto& c = counters();
    ++c.checks;
    if (passed) {
        return;
    }
    ++c.failed_checks;
    c.current_failed = true;
    std::cout << "    " << file << ':' << line << ": check failed: " << expression << '\n';
    if (!detail.empty()) {
        std::cout << "        " << detail << '\n';
    }
}

int run_all(std::string_view filter, std::ostream& out) {
    int ran = 0;
    int failed = 0;
    for (const auto& test : registry()) {
        if (!filter.empty() && test.name.find(filter) == std::string::npos) {
            continue;
        }
        ++ran;
        counters().current_failed = false;
        out << "[ RUN  ] " << test.name << '\n';
        try {
            test.body();
        } catch (const RequireFailed&) {
            // already reported by REQUIRE
        } catch (const std::exception& error) {
            counters().current_failed = true;
            out << "    unexpected exception: " << error.what() << '\n';
        } catch (...) {
            counters().current_failed = true;
            out << "    unexpected non-standard exception\n";
        }
        if (counters().current_failed) {
            ++failed;
            out << "[ FAIL ] " << test.name << "  (" << test.file << ':' << test.line << ")\n";
        } else {
            out << "[  OK  ] " << test.name << '\n';
        }
    }
    out << '\n' << ran << " test case(s), " << counters().checks << " check(s): ";
    if (ran == 0) {
        out << "no test matched the filter\n";
        return 1;
    }
    if (failed == 0) {
        out << "all passed\n";
        return 0;
    }
    out << failed << " test case(s) FAILED\n";
    return 1;
}

}  // namespace cppm::testing

int main(int argc, char** argv) {
    std::string_view filter;
    if (argc > 1) {
        const std::string_view argument = argv[1];
        if (argument == "--list") {
            for (const auto& test : cppm::testing::registry()) {
                std::cout << test.name << '\n';
            }
            return 0;
        }
        filter = argument;
    }
    return cppm::testing::run_all(filter, std::cout);
}
