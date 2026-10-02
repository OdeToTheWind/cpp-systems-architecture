// budget/store.hpp – an append-only ledger file with checksums and torn-tail recovery (Days 41, 82).
#pragma once

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "budget/money.hpp"

namespace cppm::day100 {

struct Entry {
    int id = 0;
    std::string date;  // YYYY-MM-DD
    Cents cents = 0;   // negative: expense
    std::string category;
    std::string note;
    std::string month() const { return date.substr(0, 7); }
};

inline bool valid_date(const std::string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (const std::size_t i : {0u, 1u, 2u, 3u, 5u, 6u, 8u, 9u}) {
        if (d[i] < '0' || d[i] > '9') return false;
    }
    const int y = std::stoi(d.substr(0, 4));
    const int m = std::stoi(d.substr(5, 2));
    const int day = std::stoi(d.substr(8, 2));
    const int days[] = {31, (y % 4 == 0 && y % 100 != 0) || y % 400 == 0 ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return m >= 1 && m <= 12 && day >= 1 && day <= days[m - 1];
}

inline std::uint32_t checksum(const std::string& s) {
    std::uint32_t h = 2166136261u;
    for (const char c : s) h = (h ^ static_cast<unsigned char>(c)) * 16777619u;
    return h;
}

class Ledger {
public:
    explicit Ledger(std::filesystem::path path) : path_(std::move(path)) { load(); }

    Entry add(Entry e) {
        if (!valid_date(e.date)) throw std::invalid_argument("not a date: '" + e.date + "' (use YYYY-MM-DD)");
        if (e.cents == 0) throw std::invalid_argument("amount must not be zero");
        if (e.category.empty() || e.category.find_first_of("|\n") != std::string::npos) throw std::invalid_argument("bad category");
        for (auto& c : e.note) {
            if (c == '|' || c == '\n') c = ' ';
        }
        e.id = next_id_++;
        const std::string payload = std::to_string(e.id) + "|" + e.date + "|" + std::to_string(e.cents) + "|" + e.category + "|" + e.note;
        char crc[9];
        std::snprintf(crc, sizeof crc, "%08x", static_cast<unsigned>(checksum(payload)));
        std::ofstream out(path_, std::ios::binary | std::ios::app);
        out << crc << '|' << payload << '\n';
        if (!out.flush()) throw std::runtime_error("cannot write " + path_.string());
        entries_.push_back(e);
        return e;
    }
    const std::vector<Entry>& entries() const { return entries_; }
    std::size_t recovered_bytes() const { return dropped_; }

private:
    void load() {
        std::ifstream in(path_, std::ios::binary);
        std::uintmax_t good = 0;
        for (std::string line; std::getline(in, line);) {
            if (in.eof() || line.size() < 10 || line[8] != '|') break;  // torn or damaged tail
            const std::string payload = line.substr(9);
            char crc[9];
            std::snprintf(crc, sizeof crc, "%08x", static_cast<unsigned>(checksum(payload)));
            if (line.compare(0, 8, crc) != 0) break;
            std::istringstream f(payload);
            Entry e;
            std::string id;
            std::string cents;
            std::getline(f, id, '|');
            std::getline(f, e.date, '|');
            std::getline(f, cents, '|');
            std::getline(f, e.category, '|');
            std::getline(f, e.note);
            e.id = std::stoi(id);
            e.cents = std::stoll(cents);
            entries_.push_back(e);
            next_id_ = std::max(next_id_, e.id + 1);
            good += line.size() + 1;
        }
        in.close();
        if (std::filesystem::exists(path_) && std::filesystem::file_size(path_) > good) {
            dropped_ = static_cast<std::size_t>(std::filesystem::file_size(path_) - good);
            std::filesystem::resize_file(path_, good);
        }
    }
    std::filesystem::path path_;
    std::vector<Entry> entries_;
    int next_id_ = 1;
    std::size_t dropped_ = 0;
};

}  // namespace cppm::day100
