// budget/config.hpp – INI settings with environment overrides (Days 77, 91).
#pragma once

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "budget/money.hpp"

namespace cppm::day100 {

struct Settings {
    std::string symbol = "€";
    std::string data_file = "budget.ledger";
    std::string log_file;                         // empty: no log
    std::map<std::string, Cents> monthly_limits;  // category -> limit
};

inline std::string trim(const std::string& s) {
    const auto a = s.find_first_not_of(" \t\r");
    return a == std::string::npos ? "" : s.substr(a, s.find_last_not_of(" \t\r") - a + 1);
}

/// [general] symbol, data_file, log_file and [limits] <category> = <amount>; then BUDGET_DATA_FILE,
/// BUDGET_SYMBOL and BUDGET_LOG_FILE from the environment win. Errors name the line.
inline Settings load_settings(const std::string& ini, const std::map<std::string, std::string>& env) {
    Settings s;
    std::istringstream in(ini);
    std::string section;
    int number = 0;
    for (std::string raw; std::getline(in, raw);) {
        ++number;
        const std::string line = trim(raw.substr(0, raw.find('#')));
        if (line.empty()) continue;
        if (line.front() == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            throw std::runtime_error("config line " + std::to_string(number) + ": expected key = value");
        const std::string key = trim(line.substr(0, eq));
        const std::string value = trim(line.substr(eq + 1));
        if (section == "general" && key == "symbol")
            s.symbol = value;
        else if (section == "general" && key == "data_file")
            s.data_file = value;
        else if (section == "general" && key == "log_file")
            s.log_file = value;
        else if (section == "limits") {
            try {
                s.monthly_limits[key] = parse_money(value);
            } catch (const std::invalid_argument& e) {
                throw std::runtime_error("config line " + std::to_string(number) + ": " + e.what());
            }
        } else {
            throw std::runtime_error("config line " + std::to_string(number) + ": unknown setting " + section + "." +
                                     key);
        }
    }
    if (const auto it = env.find("BUDGET_DATA_FILE"); it != env.end()) s.data_file = it->second;
    if (const auto it = env.find("BUDGET_SYMBOL"); it != env.end()) s.symbol = it->second;
    if (const auto it = env.find("BUDGET_LOG_FILE"); it != env.end()) s.log_file = it->second;
    return s;
}

}  // namespace cppm::day100
