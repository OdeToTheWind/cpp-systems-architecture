#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "lesson.hpp"

// With arguments it is the real tool (`day_100_portfolio_capstone report 2026-06`); without, the demo.
int main(int argc, char** argv) {
    if (argc > 1) {
        std::map<std::string, std::string> env;
        for (const char* name : {"BUDGET_DATA_FILE", "BUDGET_SYMBOL", "BUDGET_LOG_FILE"}) {
            if (const char* value = std::getenv(name)) env[name] = value;
        }
        return cppm::day100::run_cli({argv + 1, argv + argc}, env, std::cout, std::cerr);
    }
    return cppm::day100::run(std::cin, std::cout);
}
