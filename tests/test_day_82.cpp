// Tests for Day 82 – Building a Storage Engine. Every test works in its own temporary directory;
// crashes are simulated by damaging the log file directly.
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_82_storage_engine/lesson.hpp"

using namespace cppm::day82;

TEST_CASE("records round-trip and corruption is detected") {
    const Record r{'S', 7, "seat|12A", "Ada\nLovelace \\o/"};
    const std::string line = encode_record(r);
    CHECK_EQ(line.back(), '\n');
    const auto back = decode_record(line.substr(0, line.size() - 1));
    CHECK(back.has_value());
    CHECK_EQ(back->key, "seat|12A");
    CHECK_EQ(back->value, "Ada\nLovelace \\o/");
    CHECK_EQ(back->tx, 7u);
    std::string damaged = line.substr(0, line.size() - 1);
    damaged[12] = 'X';
    CHECK(!decode_record(damaged).has_value());
}

TEST_CASE("the store appends, indexes and survives a reopen") {
    cppm::TempDir dir("day82");
    const auto file = dir.path() / "store.log";
    {
        KvStore store = KvStore::open(file);
        store.set("user:1", "ada");
        store.set("user:2", "grace");
        store.set("user:1", "ada lovelace");  // newer record wins
        store.erase("user:2");
        CHECK_EQ(store.get("user:1").value_or(""), "ada lovelace");
        CHECK(!store.get("user:2").has_value());
        CHECK_EQ(store.log_records(), 4u);  // nothing is overwritten in place
    }
    KvStore reopened = KvStore::open(file);
    CHECK_EQ(reopened.size(), 1u);
    CHECK_EQ(reopened.get("user:1").value_or(""), "ada lovelace");
}

TEST_CASE("a torn final write is discarded on recovery") {
    cppm::TempDir dir("day82");
    const auto file = dir.path() / "store.log";
    {
        KvStore store = KvStore::open(file);
        store.set("a", "1");
        store.set("b", "2");
    }
    const auto good_size = std::filesystem::file_size(file);
    std::ofstream(file, std::ios::binary | std::ios::app)
        << encode_record({'S', 0, "c", "3"}).substr(0, 12);  // crash mid-write
    KvStore store = KvStore::open(file);
    CHECK_EQ(store.size(), 2u);
    CHECK_EQ(store.recovery().discarded_bytes, 12u);
    CHECK_EQ(std::filesystem::file_size(file), good_size);  // truncated back to the last good record
    store.set("c", "3");
    CHECK_EQ(KvStore::open(file).get("c").value_or(""), "3");
}

TEST_CASE("corruption in the middle stops replay at the damaged record") {
    cppm::TempDir dir("day82");
    const auto file = dir.path() / "store.log";
    {
        KvStore store = KvStore::open(file);
        store.set("a", "1");
        store.set("b", "2");
        store.set("c", "3");
    }
    std::fstream f(file, std::ios::in | std::ios::out | std::ios::binary);
    f.seekp(static_cast<std::streamoff>(encode_record({'S', 0, "a", "1"}).size() + 3));
    f.put('#');
    f.close();
    KvStore store = KvStore::open(file);
    CHECK_EQ(store.size(), 1u);
    CHECK(store.get("a").has_value());
    CHECK(!store.get("c").has_value());  // after the damage nothing can be trusted
}

TEST_CASE("transactions apply all of their changes or none") {
    cppm::TempDir dir("day82");
    const auto file = dir.path() / "store.log";
    {
        KvStore store = KvStore::open(file);
        store.begin().set("seat:12A", "ada").set("booking:ada", "12A").commit();
        Transaction tx = store.begin();
        tx.set("seat:14C", "grace").set("booking:grace", "14C");
        tx.crash_after(1);  // the seat record hit the disk, the booking and commit marker did not
        CHECK(!store.get("seat:14C").has_value());
    }
    KvStore store = KvStore::open(file);
    CHECK_EQ(store.get("booking:ada").value_or(""), "12A");
    CHECK(!store.get("seat:14C").has_value());
    CHECK_EQ(store.recovery().uncommitted_records, 1u);
    store.begin().set("seat:15D", "linus").commit();  // transaction ids keep increasing after a reopen
    CHECK_EQ(KvStore::open(file).get("seat:15D").value_or(""), "linus");
}

TEST_CASE("compaction keeps live data and shrinks the log") {
    cppm::TempDir dir("day82");
    const auto file = dir.path() / "store.log";
    KvStore store = KvStore::open(file);
    for (int i = 0; i < 50; ++i) store.set("counter", std::to_string(i));
    store.set("gone", "x");
    store.erase("gone");
    const auto before = store.file_bytes();
    store.compact();
    CHECK_EQ(store.log_records(), 1u);
    CHECK(store.file_bytes() < before / 10);
    CHECK_EQ(store.get("counter").value_or(""), "49");
    CHECK(!std::filesystem::exists(file.string() + ".compact"));
    CHECK_EQ(KvStore::open(file).get("counter").value_or(""), "49");
}

TEST_CASE("run keeps data between sessions") {
    cppm::TempDir dir("day82");
    const auto file = dir.path() / "demo.log";
    std::istringstream first("set theme dark mode\nbook 12A ada\nset theme light\ncompact\nstop\n");
    std::ostringstream out1;
    CHECK_EQ(run(first, out1, file), 0);
    CHECK(out1.str().find("compacted") != std::string::npos);
    std::istringstream second("get theme\nget booking:ada\nstats\nstop\n");
    std::ostringstream out2;
    run(second, out2, file);
    CHECK(out2.str().find("3 key(s), 3 record(s) replayed") != std::string::npos);
    CHECK(out2.str().find("theme = light") != std::string::npos);
    CHECK(out2.str().find("booking:ada = 12A") != std::string::npos);
}
