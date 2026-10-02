/**
 * @file
 * Day 90 – Capstone: Large File Processor.
 *
 * Scenario: a *web-server access log* far larger than the memory of the machine that must analyse
 * it. The file is read in fixed-size chunks, lines that straddle chunk boundaries are reassembled,
 * statistics are computed in one streaming pass, and the log is sorted by response time with an
 * external merge sort: sorted runs that fit in memory are written to temporary files and then
 * merged with a priority queue.
 *
 * Deliverables (syllabus):
 * - Reading in chunks
 * - Reassembling lines across chunk boundaries
 * - Streaming aggregation
 * - External merge sort
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <istream>
#include <map>
#include <ostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day90 {

namespace fs = std::filesystem;

inline constexpr Deliverable DELIVERABLES[] = {
    {"carrying partial lines from one chunk to the next", "LineAssembler::feed"},
    {"reading a stream in fixed-size chunks", "for_each_line"},
    {"one-pass statistics in constant memory", "summarise_log"},
    {"writing sorted runs that fit in memory", "write_sorted_runs"},
    {"a k-way merge with a priority queue", "merge_runs"},
};

/// Turns arbitrary chunks of bytes into complete lines. Whatever follows the last '\n' of a chunk
/// is kept and prefixed to the next chunk; finish() returns the final line without a newline.
class LineAssembler {
  public:
    template <typename OnLine>
    void feed(std::string_view chunk, OnLine&& on_line) {
        std::size_t start = 0;
        for (std::size_t nl; (nl = chunk.find('\n', start)) != std::string_view::npos; start = nl + 1) {
            if (partial_.empty()) {
                emit(chunk.substr(start, nl - start), on_line);
            } else {
                partial_.append(chunk.substr(start, nl - start));
                emit(partial_, on_line);
                partial_.clear();
            }
        }
        partial_.append(chunk.substr(start));
        max_partial_ = std::max(max_partial_, partial_.size());
    }
    template <typename OnLine>
    void finish(OnLine&& on_line) {
        if (!partial_.empty()) emit(partial_, on_line);
        partial_.clear();
    }
    std::size_t max_partial() const { return max_partial_; }

  private:
    template <typename OnLine>
    static void emit(std::string_view line, OnLine& on_line) {
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);  // Windows line endings
        on_line(line);
    }
    std::string partial_;
    std::size_t max_partial_ = 0;
};

/// Call @p on_line for every line of @p in, reading @p chunk_size bytes at a time.
/// Returns the number of chunks read.
inline std::size_t for_each_line(std::istream& in, std::size_t chunk_size,
                                 const std::function<void(std::string_view)>& on_line) {
    if (chunk_size == 0) throw std::invalid_argument("chunk size must be positive");
    std::string buffer(chunk_size, '\0');
    LineAssembler assembler;
    std::size_t chunks = 0;
    while (in.read(buffer.data(), static_cast<std::streamsize>(chunk_size)) || in.gcount() > 0) {
        ++chunks;
        assembler.feed(std::string_view(buffer.data(), static_cast<std::size_t>(in.gcount())), on_line);
    }
    assembler.finish(on_line);
    return chunks;
}

/// A log line: "<ip> <method> <path> <status> <ms>".
struct LogStats {
    std::size_t lines = 0;
    std::size_t malformed = 0;
    std::map<int, std::size_t> by_status;
    long long total_ms = 0;
    int slowest_ms = 0;
    std::string slowest_path;
};

/// Everything in one pass; memory use does not depend on the file size.
inline LogStats summarise_log(std::istream& in, std::size_t chunk_size) {
    LogStats stats;
    for_each_line(in, chunk_size, [&stats](std::string_view line) {
        if (line.empty()) return;
        ++stats.lines;
        std::istringstream fields{std::string(line)};
        std::string ip;
        std::string method;
        std::string path;
        int status = 0;
        int ms = 0;
        if (!(fields >> ip >> method >> path >> status >> ms)) {
            ++stats.malformed;
            return;
        }
        ++stats.by_status[status];
        stats.total_ms += ms;
        if (ms > stats.slowest_ms) {
            stats.slowest_ms = ms;
            stats.slowest_path = path;
        }
    });
    return stats;
}

using LineLess = std::function<bool(const std::string&, const std::string&)>;

/// Phase 1 of the external sort: read at most @p max_lines lines at a time, sort them, write each
/// batch to its own run file in @p temp_dir. Returns the run files.
inline std::vector<fs::path> write_sorted_runs(std::istream& in, std::size_t max_lines, const fs::path& temp_dir,
                                               const LineLess& less) {
    if (max_lines == 0) throw std::invalid_argument("max_lines must be positive");
    std::vector<fs::path> runs;
    std::vector<std::string> batch;
    auto flush = [&] {
        if (batch.empty()) return;
        std::stable_sort(batch.begin(), batch.end(), less);
        runs.push_back(temp_dir / ("run-" + std::to_string(runs.size()) + ".txt"));
        std::ofstream out(runs.back(), std::ios::binary);
        for (const auto& line : batch) out << line << '\n';
        if (!out) throw std::runtime_error("cannot write " + runs.back().string());
        batch.clear();
    };
    for_each_line(in, 64 * 1024, [&](std::string_view line) {
        if (line.empty()) return;
        batch.emplace_back(line);
        if (batch.size() == max_lines) flush();
    });
    flush();
    return runs;
}

/// Phase 2: merge all runs at once. The priority queue holds one line per run – the smallest
/// remaining line of each – so memory is proportional to the number of runs, not the file size.
inline std::size_t merge_runs(const std::vector<fs::path>& runs, std::ostream& out, const LineLess& less) {
    struct Head {
        std::string line;
        std::size_t run;
    };
    auto greater = [&less](const Head& a, const Head& b) {
        if (less(b.line, a.line)) return true;
        if (less(a.line, b.line)) return false;
        return a.run > b.run;  // ties: earlier run first, so the sort is stable
    };
    std::priority_queue<Head, std::vector<Head>, decltype(greater)> heap(greater);
    std::vector<std::ifstream> files;
    for (const auto& r : runs) files.emplace_back(r, std::ios::binary);
    for (std::size_t i = 0; i < files.size(); ++i) {
        std::string line;
        if (std::getline(files[i], line)) heap.push({line, i});
    }
    std::size_t written = 0;
    while (!heap.empty()) {
        Head head = heap.top();
        heap.pop();
        out << head.line << '\n';
        ++written;
        if (std::getline(files[head.run], head.line)) heap.push(head);
    }
    return written;
}

struct SortReport {
    std::size_t runs = 0;
    std::size_t lines = 0;
};

inline SortReport external_sort(std::istream& in, std::ostream& out, std::size_t max_lines, const fs::path& temp_dir,
                                const LineLess& less) {
    fs::create_directories(temp_dir);
    const auto runs = write_sorted_runs(in, max_lines, temp_dir, less);
    const std::size_t lines = merge_runs(runs, out, less);
    for (const auto& r : runs) fs::remove(r);
    return {runs.size(), lines};
}

/// Order by the response time (last field), slowest first; unparsable lines go last.
inline bool slower_first(const std::string& a, const std::string& b) {
    auto ms = [](const std::string& line) {
        const auto space = line.find_last_of(' ');
        try {
            return space == std::string::npos ? -1 : std::stoi(line.substr(space + 1));
        } catch (const std::exception&) {
            return -1;
        }
    };
    return ms(a) > ms(b);
}

/// A deterministic synthetic access log of @p n lines.
inline std::string synthetic_log(int n) {
    static const char* const paths[] = {"/", "/login", "/api/items", "/api/cart", "/static/app.js"};
    std::string log;
    unsigned seed = 90;
    for (int i = 0; i < n; ++i) {
        seed = seed * 1103515245u + 12345u;
        const unsigned r = (seed >> 8) % 10000;
        const int status = r % 50 == 0 ? 500 : r % 10 == 0 ? 404 : 200;
        log += "10.0.0." + std::to_string(r % 250) + " GET " + paths[r % 5] + " " + std::to_string(status) + " " +
               std::to_string(5 + r % 900) + "\n";
    }
    return log;
}

/// The interactive demo: "<lines> <chunk_bytes> <max_lines_in_memory>" analyses and sorts a synthetic log.
inline int run(std::istream& in, std::ostream& out, const fs::path& temp = fs::temp_directory_path() / "cppm-extsort") {
    out << "Day 90 – Capstone: Large File Processor\n";
    while (auto line = prompt_line(in, out, "lines chunk_bytes max_lines> ")) {
        std::istringstream words(*line);
        int n = 0;
        std::size_t chunk = 0;
        std::size_t max_lines = 0;
        if (!(words >> n >> chunk >> max_lines) || n <= 0 || chunk == 0 || max_lines == 0) break;
        const std::string log = synthetic_log(n);
        std::istringstream for_stats(log);
        auto stats = summarise_log(for_stats, chunk);  // non-const: operator[] fills in missing codes
        out << "  " << stats.lines << " line(s): 200 x" << stats.by_status[200] << ", 404 x" << stats.by_status[404]
            << ", 500 x" << stats.by_status[500] << "; slowest " << stats.slowest_ms << " ms on " << stats.slowest_path
            << '\n';
        std::istringstream for_sort(log);
        std::ostringstream sorted;
        const auto report = external_sort(for_sort, sorted, max_lines, temp, slower_first);
        out << "  sorted with " << report.runs << " run(s); top: " << sorted.str().substr(0, sorted.str().find('\n'))
            << '\n';
    }
    return 0;
}

}  // namespace cppm::day90
