/**
 * @file
 * Day 85 – Capstone: Concurrent File Processor.
 *
 * Scenario: an *integrity checker for a photo archive*. Thousands of files sit on a NAS; once a
 * week every file is checksummed in parallel and compared with the manifest from last time, so
 * silent corruption, deleted files and unexpected new files are reported before the backups
 * rotate. Results are deterministic – sorted by path – however the threads were scheduled.
 *
 * Deliverables (syllabus):
 * - Walking a directory tree
 * - Checksums computed in parallel
 * - Worker threads pulling from a shared index
 * - Manifests and verification reports
 */
#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day85 {

namespace fs = std::filesystem;

inline constexpr Deliverable DELIVERABLES[] = {
    {"CRC-32 computed over a stream in chunks", "crc32"},
    {"a sorted, recursive file listing", "list_files"},
    {"workers claiming files through an atomic index", "checksum_all"},
    {"writing and reading a manifest", "read_manifest"},
    {"comparing the archive with its manifest", "verify"},
};

inline constexpr const char* MANIFEST_NAME = "MANIFEST.crc32";

/// CRC-32 (IEEE, as in zip and PNG), table computed once.
inline std::uint32_t crc32(std::istream& in) {
    static const auto table = [] {
        std::array<std::uint32_t, 256> t{};
        for (std::uint32_t i = 0; i < 256; ++i) {
            std::uint32_t c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            t[i] = c;
        }
        return t;
    }();
    std::uint32_t crc = 0xFFFFFFFFu;
    std::array<char, 64 * 1024> buffer{};  // stream in chunks: files may be larger than memory
    while (in) {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto got = static_cast<std::size_t>(in.gcount());
        for (std::size_t i = 0; i < got; ++i)
            crc = table[(crc ^ static_cast<unsigned char>(buffer[i])) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

inline std::uint32_t crc32_of(const std::string& data) {
    std::istringstream in(data);
    return crc32(in);
}

/// Regular files under @p root as generic ("a/b.jpg") relative paths, sorted; the manifest itself is skipped.
inline std::vector<std::string> list_files(const fs::path& root) {
    std::vector<std::string> files;
    for (const auto& entry : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied)) {
        if (!entry.is_regular_file()) continue;
        const std::string rel = fs::relative(entry.path(), root).generic_string();
        if (rel != MANIFEST_NAME) files.push_back(rel);
    }
    std::sort(files.begin(), files.end());
    return files;
}

struct FileResult {
    std::string path;
    std::uintmax_t size = 0;
    std::uint32_t crc = 0;
    std::string error;  // empty when the file was read
};

/// Checksum @p files with @p threads workers. Each worker repeatedly claims the next index with
/// fetch_add, so fast workers take more files and no file is processed twice. Every result goes to
/// its own pre-allocated slot, so no lock is needed and the output order matches the input order.
inline std::vector<FileResult> checksum_all(const fs::path& root, const std::vector<std::string>& files,
                                            unsigned threads) {
    if (threads == 0) throw std::invalid_argument("need at least one thread");
    std::vector<FileResult> results(files.size());
    std::atomic<std::size_t> next{0};
    auto worker = [&] {
        for (std::size_t i = next.fetch_add(1); i < files.size(); i = next.fetch_add(1)) {
            FileResult& r = results[i];
            r.path = files[i];
            std::ifstream in(root / files[i], std::ios::binary);
            if (!in) {
                r.error = "cannot open";
                continue;
            }
            r.crc = crc32(in);
            if (in.bad()) r.error = "read error";
            std::error_code ec;
            r.size = fs::file_size(root / files[i], ec);
        }
    };
    std::vector<std::thread> pool;
    for (unsigned t = 1; t < threads; ++t) pool.emplace_back(worker);
    worker();  // the calling thread works too
    for (auto& t : pool) t.join();
    return results;
}

inline std::string hex8(std::uint32_t v) {
    char text[9];
    std::snprintf(text, sizeof text, "%08x", static_cast<unsigned>(v));
    return text;
}

/// "crc32 size path" per line, like sha256sum but with the size as a cheap first check.
inline void write_manifest(std::ostream& out, const std::vector<FileResult>& results) {
    for (const auto& r : results) {
        if (r.error.empty()) out << hex8(r.crc) << ' ' << r.size << ' ' << r.path << '\n';
    }
}

inline std::map<std::string, FileResult> read_manifest(std::istream& in) {
    std::map<std::string, FileResult> entries;
    int number = 0;
    for (std::string line; std::getline(in, line);) {
        ++number;
        std::istringstream fields(line);
        std::string crc;
        FileResult r;
        if (!(fields >> crc >> r.size) || crc.size() != 8 || !std::getline(fields >> std::ws, r.path) ||
            r.path.empty()) {
            throw std::runtime_error("manifest line " + std::to_string(number) + " is malformed");
        }
        r.crc = static_cast<std::uint32_t>(std::stoul(crc, nullptr, 16));
        entries[r.path] = r;
    }
    return entries;
}

struct VerifyReport {
    std::vector<std::string> ok;
    std::vector<std::string> modified;
    std::vector<std::string> missing;
    std::vector<std::string> added;
    std::vector<std::string> unreadable;
    bool clean() const { return modified.empty() && missing.empty() && added.empty() && unreadable.empty(); }
};

inline VerifyReport verify(const fs::path& root, const std::map<std::string, FileResult>& manifest, unsigned threads) {
    VerifyReport report;
    std::map<std::string, bool> seen;
    for (const auto& r : checksum_all(root, list_files(root), threads)) {
        if (!r.error.empty()) {
            report.unreadable.push_back(r.path);
            continue;
        }
        const auto it = manifest.find(r.path);
        if (it == manifest.end())
            report.added.push_back(r.path);
        else if (it->second.crc != r.crc || it->second.size != r.size)
            report.modified.push_back(r.path);
        else
            report.ok.push_back(r.path);
        seen[r.path] = true;
    }
    for (const auto& [path, entry] : manifest) {
        if (!seen.contains(path)) report.missing.push_back(path);
    }
    return report;
}

/// Create the manifest file in @p root and return how many files it lists.
inline std::size_t create_manifest(const fs::path& root, unsigned threads) {
    const auto results = checksum_all(root, list_files(root), threads);
    std::ofstream out(root / MANIFEST_NAME, std::ios::binary | std::ios::trunc);
    write_manifest(out, results);
    return results.size();
}

/// The interactive demo builds a small archive in @p root, then applies commands:
/// "manifest", "verify", "touch <file> <text>", "rm <file>".
inline int run(std::istream& in, std::ostream& out, const fs::path& root = fs::temp_directory_path() / "cppm-archive") {
    out << "Day 85 – Capstone: Concurrent File Processor\n";
    fs::create_directories(root / "2026" / "06");
    for (int i = 1; i <= 6; ++i)
        std::ofstream(root / "2026" / "06" / ("IMG_" + std::to_string(i) + ".jpg"), std::ios::binary)
            << std::string(1000u * static_cast<unsigned>(i), static_cast<char>('a' + i));
    const unsigned threads = std::max(2u, std::thread::hardware_concurrency());
    while (auto line = prompt_line(in, out, "manifest | verify | touch <f> <text> | rm <f>> ")) {
        std::istringstream words(*line);
        std::string command;
        std::string file;
        if (!(words >> command)) break;
        words >> file;
        if (command == "manifest") {
            out << "  manifest lists " << create_manifest(root, threads) << " file(s)\n";
        } else if (command == "verify") {
            std::ifstream manifest_file(root / MANIFEST_NAME);
            const auto report = verify(root, read_manifest(manifest_file), threads);
            out << "  ok " << report.ok.size() << ", modified " << report.modified.size() << ", missing "
                << report.missing.size() << ", added " << report.added.size()
                << (report.clean() ? " – archive intact" : "") << '\n';
            for (const auto& p : report.modified) out << "  MODIFIED " << p << '\n';
            for (const auto& p : report.missing) out << "  MISSING  " << p << '\n';
            for (const auto& p : report.added) out << "  ADDED    " << p << '\n';
        } else if (command == "touch") {
            std::string text;
            std::getline(words >> std::ws, text);
            std::ofstream(root / file, std::ios::binary | std::ios::app) << text;
        } else if (command == "rm") {
            fs::remove(root / file);
        }
    }
    return 0;
}

}  // namespace cppm::day85
