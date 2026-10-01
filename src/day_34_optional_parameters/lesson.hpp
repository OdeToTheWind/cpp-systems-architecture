/**
 * @file
 * Day 34 – Optional, Required & Default Parameters.
 *
 * Scenario: a *continuous-integration job scheduler*. A job always needs a name and a
 * command; everything else – timeout, branch filter, retries, build matrix – is optional,
 * and "not given" must stay distinguishable from "given as zero".
 *
 * Deliverables (syllabus):
 * - std::optional parameters
 * - Overload sets
 * - Builder objects
 * - Parameter ordering rules
 */
#pragma once

#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day34 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"required parameters first, defaults last", "schedule_job"},
    {"std::optional tells 'not given' apart from 'given as 0'", "effective_retries"},
    {"an overload for a different shape of input", "schedule_matrix"},
    {"a builder for many optional settings", "JobBuilder"},
    {"validation of the combined settings", "validate"},
};

inline constexpr int project_default_retries = 2;

/// A fully resolved job.
struct Job {
    std::string name;
    std::string command;
    int timeout_minutes;
    std::string branch;  // "*" means every branch
    int retries;
    std::vector<std::string> environment;
};

/// nullopt means "use the project default"; 0 means "really, no retries".
inline int effective_retries(std::optional<int> requested) {
    if (requested && *requested < 0) {
        throw std::invalid_argument("retries cannot be negative");
    }
    return requested.value_or(project_default_retries);
}

/// Every rule a job must satisfy, whichever way it was created.
inline void validate(const Job& job) {
    if (job.name.empty() || job.command.empty()) {
        throw std::invalid_argument("name and command are required");
    }
    if (job.timeout_minutes < 1 || job.timeout_minutes > 360) {
        throw std::invalid_argument("timeout must be 1-360 minutes");
    }
    if (job.retries > 5) {
        throw std::invalid_argument("at most 5 retries");
    }
}

/// Required parameters come first; a parameter with a default may only be followed by others
/// with defaults (C++ fills arguments strictly left to right).
inline Job schedule_job(std::string name, std::string command, int timeout_minutes = 30,
                        std::optional<std::string> branch = std::nullopt, std::optional<int> retries = std::nullopt) {
    Job job{std::move(name), std::move(command), timeout_minutes, branch.value_or("*"), effective_retries(retries), {}};
    validate(job);
    return job;
}

/// Overload: the same job once per environment of a build matrix.
inline std::vector<Job> schedule_matrix(const std::string& name, const std::string& command,
                                        const std::vector<std::string>& environments, int timeout_minutes = 30) {
    if (environments.empty()) {
        throw std::invalid_argument("a matrix needs at least one environment");
    }
    std::vector<Job> jobs;
    for (const auto& environment : environments) {
        Job job = schedule_job(name + " (" + environment + ")", command, timeout_minutes);
        job.environment = {environment};
        jobs.push_back(std::move(job));
    }
    return jobs;
}

/// Required values go to the constructor; optional ones have named setters; build() validates once.
class JobBuilder {
public:
    JobBuilder(std::string name, std::string command) : job_{std::move(name), std::move(command), 30, "*", project_default_retries, {}} {}
    JobBuilder& timeout(int minutes) {
        job_.timeout_minutes = minutes;
        return *this;
    }
    JobBuilder& only_on(std::string branch) {
        job_.branch = std::move(branch);
        return *this;
    }
    JobBuilder& retries(int count) {
        job_.retries = effective_retries(count);
        return *this;
    }
    JobBuilder& env(std::string variable) {
        job_.environment.push_back(std::move(variable));
        return *this;
    }
    Job build() const {
        validate(job_);
        return job_;
    }

private:
    Job job_;
};

inline std::string describe(const Job& job) {
    std::ostringstream out;
    out << job.name << ": `" << job.command << "` on " << job.branch << ", " << job.timeout_minutes << " min, "
        << job.retries << " retr" << (job.retries == 1 ? "y" : "ies");
    return out.str();
}

/// The interactive demo: "name command [timeout] [branch] [retries]" (use - to skip a value).
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 34 – Optional, Required & Default Parameters\n";
    for (const auto& job : schedule_matrix("unit-tests", "ctest", {"gcc", "clang"})) {
        out << "  " << describe(job) << '\n';
    }
    while (auto line = prompt_line(in, out, "name command [timeout|-] [branch|-] [retries|-]> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        std::string name;
        std::string command;
        std::string timeout = "-";
        std::string branch = "-";
        std::string retries = "-";
        words >> name >> command >> timeout >> branch >> retries;
        try {
            const Job job = schedule_job(name, command, timeout == "-" ? 30 : std::stoi(timeout),
                                         branch == "-" ? std::nullopt : std::optional<std::string>(branch),
                                         retries == "-" ? std::nullopt : std::optional<int>(std::stoi(retries)));
            out << "  " << describe(job) << '\n';
        } catch (const std::exception& error) {
            out << "  rejected: " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day34
