/**
 * @file
 * Day 41 – File I/O with fstream.
 *
 * Scenario: a *flight data recorder for a hobby drone*. Events are appended to a text log a
 * human can read, telemetry samples are stored in a compact binary file with a fixed,
 * portable record layout, and every read checks for missing, unreadable or truncated files.
 *
 * Deliverables (syllabus):
 * - ifstream and ofstream
 * - Text and binary files
 * - Append mode
 * - Error checking
 * - RAII file handling
 */
#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day41 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"appending text lines with std::ios::app", "append_event"},
    {"reading a text file line by line with ifstream", "read_events"},
    {"writing fixed-size binary records portably", "write_samples"},
    {"reading binary records and detecting truncation", "read_samples"},
    {"checking every stream operation for errors", "measured_size"},
};

/// One telemetry sample. Stored as 10 bytes in little-endian order, independent of struct padding.
struct Sample {
    std::uint32_t time_ms;
    std::int32_t altitude_cm;
    std::uint16_t battery_mv;
    bool operator==(const Sample&) const = default;
};
inline constexpr std::size_t record_size = 4 + 4 + 2;

/// Append one line to the event log. The ofstream closes itself when it goes out of scope (RAII).
inline void append_event(const std::filesystem::path& log, const std::string& event) {
    std::ofstream out(log, std::ios::app);  // app: every write goes to the end; existing lines are kept
    if (!out) {
        throw std::runtime_error("cannot open " + log.string() + " for appending");
    }
    out << event << '\n';
    if (!out) {
        throw std::runtime_error("write to " + log.string() + " failed");
    }
}

/// Every line of the event log, in order.
inline std::vector<std::string> read_events(const std::filesystem::path& log) {
    std::ifstream in(log);
    if (!in) {
        throw std::runtime_error("cannot open " + log.string());
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    if (in.bad()) {  // eof is expected; bad() means a real I/O error
        throw std::runtime_error("read error in " + log.string());
    }
    return lines;
}

namespace detail {
inline void put(std::ostream& out, std::uint64_t value, std::size_t bytes) {
    for (std::size_t i = 0; i < bytes; ++i) {
        out.put(static_cast<char>((value >> (8 * i)) & 0xFFu));
    }
}
inline std::uint64_t take(const std::array<unsigned char, record_size>& record, std::size_t offset, std::size_t bytes) {
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < bytes; ++i) {
        value |= static_cast<std::uint64_t>(record[offset + i]) << (8 * i);
    }
    return value;
}
}  // namespace detail

/// Overwrite @p file with the samples, field by field, little-endian – the same bytes on every machine.
inline void write_samples(const std::filesystem::path& file, const std::vector<Sample>& samples) {
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("cannot create " + file.string());
    }
    for (const Sample& s : samples) {
        detail::put(out, s.time_ms, 4);
        detail::put(out, static_cast<std::uint32_t>(s.altitude_cm), 4);
        detail::put(out, s.battery_mv, 2);
    }
    if (!out.flush()) {
        throw std::runtime_error("write to " + file.string() + " failed");
    }
}

/// Read all samples back; a file whose size is not a whole number of records is reported as truncated.
inline std::vector<Sample> read_samples(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open " + file.string());
    }
    std::vector<Sample> samples;
    std::array<unsigned char, record_size> record{};
    while (in.read(reinterpret_cast<char*>(record.data()), static_cast<std::streamsize>(record.size()))) {
        samples.push_back({static_cast<std::uint32_t>(detail::take(record, 0, 4)),
                           static_cast<std::int32_t>(static_cast<std::uint32_t>(detail::take(record, 4, 4))),
                           static_cast<std::uint16_t>(detail::take(record, 8, 2))});
    }
    if (in.gcount() != 0) {
        throw std::runtime_error(file.string() + " is truncated: " + std::to_string(in.gcount()) + " stray byte(s)");
    }
    return samples;
}

/// Size in bytes, measured by seeking to the end of the stream. (Not called `file_size`: with a
/// std::filesystem::path argument, argument-dependent lookup would also find std::filesystem::file_size.)
inline std::uintmax_t measured_size(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary | std::ios::ate);  // ate: start positioned at the end
    if (!in) {
        throw std::runtime_error("cannot open " + file.string());
    }
    const auto end = in.tellg();
    if (end < 0) {
        throw std::runtime_error("cannot determine the size of " + file.string());
    }
    return static_cast<std::uintmax_t>(end);
}

/// The interactive demo: "event <text>" or "sample <ms> <cm> <mV>" into files in @p folder.
inline int run(std::istream& in, std::ostream& out, const std::filesystem::path& folder = std::filesystem::temp_directory_path()) {
    out << "Day 41 – File I/O with fstream\nFiles in " << folder.string() << '\n';
    const auto log = folder / "flight-events.log";
    const auto data = folder / "flight-telemetry.bin";
    std::vector<Sample> samples;
    while (auto line = prompt_line(in, out, "event <text> | sample <ms> <cm> <mV>> ")) {
        std::istringstream words(*line);
        std::string kind;
        if (!(words >> kind)) {
            break;
        }
        if (kind == "event") {
            std::string text;
            std::getline(words >> std::ws, text);
            append_event(log, text);
        } else if (kind == "sample") {
            Sample s{};
            if (words >> s.time_ms >> s.altitude_cm >> s.battery_mv) {
                samples.push_back(s);
            } else {
                out << "  sample needs three numbers\n";
            }
        }
    }
    write_samples(data, samples);
    out << read_samples(data).size() << " sample(s), " << measured_size(data) << " bytes\n";
    if (std::filesystem::exists(log)) {
        out << read_events(log).size() << " event line(s) in the log\n";
    }
    return 0;
}

}  // namespace cppm::day41
