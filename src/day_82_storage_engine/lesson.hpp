/**
 * @file
 * Day 82 – Building a Storage Engine.
 *
 * Scenario: the *session store* of a ticket-booking site – a small key-value database that must
 * not lose a booking when the server crashes mid-write. Like Bitcask or a database's write-ahead
 * log, it only ever appends records to a file, keeps an in-memory index from each key to the
 * offset of its latest record, recovers from a torn final write, groups changes into
 * all-or-nothing transactions, and compacts the log when it fills with stale records.
 *
 * Deliverables (syllabus):
 * - Append-only logs
 * - In-memory indexes
 * - Crash recovery
 * - Compaction and transactions
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day82 {

namespace fs = std::filesystem;

inline constexpr Deliverable DELIVERABLES[] = {
    {"a checksummed record format", "encode_record"},
    {"appending records and indexing their offsets", "KvStore::set"},
    {"replaying the log on open and discarding a torn tail", "KvStore::open"},
    {"all-or-nothing transactions with a commit marker", "Transaction::commit"},
    {"rewriting only live keys and swapping files atomically", "KvStore::compact"},
};

/// FNV-1a: a fast checksum that detects torn or corrupted records.
inline std::uint32_t fnv1a(const std::string& data) {
    std::uint32_t hash = 2166136261u;
    for (const char c : data) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 16777619u;
    }
    return hash;
}

inline std::string escape(const std::string& text) {
    std::string out;
    for (const char c : text) {
        if (c == '\\') out += "\\\\";
        else if (c == '|') out += "\\p";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

inline std::string unescape(const std::string& text) {
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\\' || i + 1 == text.size()) {
            out += text[i];
            continue;
        }
        const char next = text[++i];
        out += next == 'p' ? '|' : next == 'n' ? '\n' : next;
    }
    return out;
}

struct Record {
    char op;            // 'S' set, 'D' delete, 'C' commit
    std::uint64_t tx;   // 0 = not in a transaction
    std::string key;
    std::string value;
};

/// One line per record: "<checksum hex>|<op>|<tx>|<key>|<value>\n" with | and newlines escaped.
inline std::string encode_record(const Record& r) {
    const std::string payload = std::string(1, r.op) + "|" + std::to_string(r.tx) + "|" + escape(r.key) + "|" + escape(r.value);
    char crc[9];
    std::snprintf(crc, sizeof crc, "%08x", static_cast<unsigned>(fnv1a(payload)));
    return std::string(crc) + "|" + payload + "\n";
}

/// nullopt for a damaged line (bad checksum or shape).
inline std::optional<Record> decode_record(const std::string& line) {
    if (line.size() < 10 || line[8] != '|') return std::nullopt;
    const std::string payload = line.substr(9);
    char expected[9];
    std::snprintf(expected, sizeof expected, "%08x", static_cast<unsigned>(fnv1a(payload)));
    if (line.compare(0, 8, expected) != 0) return std::nullopt;
    std::vector<std::string> parts;
    std::size_t start = 0;
    for (std::size_t bar; (bar = payload.find('|', start)) != std::string::npos; start = bar + 1) parts.push_back(payload.substr(start, bar - start));
    parts.push_back(payload.substr(start));
    if (parts.size() != 4 || parts[0].size() != 1) return std::nullopt;
    return Record{parts[0][0], std::stoull(parts[1]), unescape(parts[2]), unescape(parts[3])};
}

struct RecoveryReport {
    std::size_t records = 0;
    std::size_t discarded_bytes = 0;      // torn or corrupt tail removed
    std::size_t uncommitted_records = 0;  // from transactions without a commit marker
};

class KvStore;

/// Buffers changes; nothing reaches the log until commit(), which appends all of them followed by
/// a commit marker in one write. Recovery ignores a transaction whose marker is missing.
class Transaction {
public:
    Transaction(KvStore& store, std::uint64_t id) : store_(store), id_(id) {}
    Transaction& set(const std::string& key, const std::string& value) {
        records_.push_back({'S', id_, key, value});
        return *this;
    }
    Transaction& erase(const std::string& key) {
        records_.push_back({'D', id_, key, ""});
        return *this;
    }
    void commit();
    /// Simulate a crash during commit: only the first @p records reach the disk, no marker.
    void crash_after(std::size_t records);

private:
    KvStore& store_;
    std::uint64_t id_;
    std::vector<Record> records_;
};

class KvStore {
public:
    /// Open (or create) the log at @p path and rebuild the index by replaying it.
    static KvStore open(const fs::path& path) {
        KvStore store(path);
        store.report_ = store.replay();
        return store;
    }

    std::optional<std::string> get(const std::string& key) const {
        const auto it = index_.find(key);
        if (it == index_.end()) return std::nullopt;
        std::ifstream in(path_, std::ios::binary);  // read the record the index points to
        in.seekg(it->second);
        std::string line;
        std::getline(in, line);
        const auto record = decode_record(line);
        if (!record) throw std::runtime_error("index points at a damaged record for " + key);
        return record->value;
    }
    void set(const std::string& key, const std::string& value) { append({{'S', 0, key, value}}); }
    void erase(const std::string& key) { append({{'D', 0, key, ""}}); }
    Transaction begin() { return Transaction(*this, ++last_tx_); }

    /// Write only the latest value of every live key to a new file, then rename it over the old
    /// log. rename() replaces the file in one step, so a crash leaves either the old or the new log.
    void compact() {
        const fs::path temp = path_.string() + ".compact";
        {
            std::ofstream out(temp, std::ios::binary | std::ios::trunc);
            for (const auto& [key, offset] : index_) out << encode_record({'S', 0, key, *get(key)});
            out.flush();
            if (!out) throw std::runtime_error("compaction failed while writing " + temp.string());
        }
        fs::rename(temp, path_);
        report_ = replay();
    }

    std::size_t size() const { return index_.size(); }
    std::size_t log_records() const { return records_in_log_; }
    std::uintmax_t file_bytes() const { return fs::exists(path_) ? fs::file_size(path_) : 0; }
    const RecoveryReport& recovery() const { return report_; }

private:
    friend class Transaction;
    explicit KvStore(fs::path path) : path_(std::move(path)) {}

    /// Append encoded records in one write and update the index with their offsets.
    void append(const std::vector<Record>& records, bool apply = true) {
        std::ofstream out(path_, std::ios::binary | std::ios::app);
        auto offset = static_cast<std::streamoff>(file_bytes());
        std::string batch;
        std::vector<std::pair<const Record*, std::streamoff>> placed;
        for (const auto& r : records) {
            const std::string line = encode_record(r);
            placed.emplace_back(&r, offset + static_cast<std::streamoff>(batch.size()));
            batch += line;
        }
        out << batch;
        out.flush();
        if (!out) throw std::runtime_error("write failed: " + path_.string());
        records_in_log_ += records.size();
        if (!apply) return;
        for (const auto& [r, at] : placed) {
            if (r->op == 'S') index_[r->key] = at;
            else if (r->op == 'D') index_.erase(r->key);
        }
    }

    RecoveryReport replay() {
        RecoveryReport report;
        index_.clear();
        records_in_log_ = 0;
        std::ifstream in(path_, std::ios::binary);
        std::map<std::uint64_t, std::vector<std::pair<Record, std::streamoff>>> pending;  // tx -> buffered ops
        std::streamoff good_end = 0;
        for (std::string line;;) {
            const std::streamoff at = in.tellg();
            if (!std::getline(in, line)) break;
            const bool complete = !in.eof();  // a final line without '\n' was torn mid-write
            const auto record = complete ? decode_record(line) : std::nullopt;
            if (!record) break;  // stop at the first damaged record: everything after is suspect
            good_end = in.tellg();
            ++report.records;
            last_tx_ = std::max(last_tx_, record->tx);
            if (record->tx == 0) {
                apply(*record, at);
            } else if (record->op == 'C') {
                for (const auto& [r, offset] : pending[record->tx]) apply(r, offset);
                pending.erase(record->tx);
            } else {
                pending[record->tx].emplace_back(*record, at);
            }
        }
        for (const auto& [tx, ops] : pending) report.uncommitted_records += ops.size();
        in.close();
        records_in_log_ = report.records;
        const auto size = file_bytes();
        if (size > static_cast<std::uintmax_t>(good_end)) {
            report.discarded_bytes = static_cast<std::size_t>(size - static_cast<std::uintmax_t>(good_end));
            fs::resize_file(path_, static_cast<std::uintmax_t>(good_end));  // new appends start at a clean boundary
        }
        return report;
    }

    void apply(const Record& r, std::streamoff offset) {
        if (r.op == 'S') index_[r.key] = offset;
        else if (r.op == 'D') index_.erase(r.key);
    }

    fs::path path_;
    std::map<std::string, std::streamoff> index_;  // key -> offset of its latest record
    std::size_t records_in_log_ = 0;
    std::uint64_t last_tx_ = 0;
    RecoveryReport report_;
};

inline void Transaction::commit() {
    auto records = records_;
    records.push_back({'C', id_, "", ""});
    store_.append(records);
    records_.clear();
}

inline void Transaction::crash_after(std::size_t records) {
    std::vector<Record> written(records_.begin(), records_.begin() + static_cast<std::ptrdiff_t>(std::min(records, records_.size())));
    store_.append(written, false);  // reached the disk, but the process "died" before the marker
    records_.clear();
}

/// The interactive demo on a persistent log: set k v | del k | get k | book <seat> <name> | compact | stats
inline int run(std::istream& in, std::ostream& out, const fs::path& file = fs::temp_directory_path() / "cppm-sessions.log") {
    out << "Day 82 – Building a Storage Engine\n";
    KvStore store = KvStore::open(file);
    out << "opened " << file.filename().string() << ": " << store.size() << " key(s), " << store.recovery().records
        << " record(s) replayed\n";
    while (auto line = prompt_line(in, out, "set|del|get|book|compact|stats> ")) {
        std::istringstream words(*line);
        std::string command;
        std::string key;
        if (!(words >> command)) break;
        words >> key;
        std::string value;
        std::getline(words >> std::ws, value);
        if (command == "set") {
            store.set(key, value);
        } else if (command == "del") {
            store.erase(key);
        } else if (command == "get") {
            out << "  " << key << " = " << store.get(key).value_or("(missing)") << '\n';
        } else if (command == "book") {  // a booking touches two keys: both or neither
            store.begin().set("seat:" + key, value).set("booking:" + value, key).commit();
        } else if (command == "compact") {
            const auto before = store.file_bytes();
            store.compact();
            out << "  compacted " << before << " -> " << store.file_bytes() << " bytes\n";
        } else if (command == "stats") {
            out << "  " << store.size() << " key(s), " << store.log_records() << " record(s), " << store.file_bytes() << " bytes\n";
        }
    }
    return 0;
}

}  // namespace cppm::day82
