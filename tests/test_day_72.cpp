// Tests for Day 72 – Design Patterns.
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_72_design_patterns/lesson.hpp"

using namespace cppm::day72;

namespace {
const Lines groceries{"Groceries", "eggs & milk", "<bread>"};
}

TEST_CASE("strategies render the same lines differently") {
    CHECK_EQ(PlainExporter{}.render({"a", "b"}), "a\nb\n");
    CHECK_EQ(MarkdownExporter{}.render(groceries), "# Groceries\n- eggs & milk\n- <bread>\n");
    CHECK(HtmlExporter{}.render(groceries).find("<li>eggs &amp; milk</li>") != std::string::npos);
    CHECK(HtmlExporter{}.render(groceries).find("&lt;bread&gt;") != std::string::npos);
}

TEST_CASE("the factory creates exporters by name and can be extended") {
    CHECK_EQ(make_exporter("plain")->render({"x"}), "x\n");
    CHECK_THROWS_AS(make_exporter("pdf"), std::invalid_argument);
    exporter_registry()["shout"] = [] {
        struct Shout : Exporter {
            std::string render(const Lines& lines) const override { return lines.empty() ? "" : lines[0] + "!\n"; }
        };
        return std::make_unique<Shout>();
    };
    CHECK_EQ(make_exporter("shout")->render({"hi"}), "hi!\n");
    exporter_registry().erase("shout");
}

TEST_CASE("decorators stack in the order they are applied") {
    auto exporter = std::make_unique<WithWordCount>(std::make_unique<WithLineNumbers>(make_exporter("markdown")));
    CHECK_EQ(exporter->render({"Plan", "write tests"}), "# 1. Plan\n- 2. write tests\n(3 words)\n");
}

TEST_CASE("commands undo exactly what they did") {
    Lines lines{"a", "b", "c"};
    DeleteLine del(1);
    del.execute(lines);
    CHECK(lines == Lines{"a", "c"});
    del.undo(lines);
    CHECK(lines == Lines{"a", "b", "c"});
    ReplaceAll replace("a", "aa");
    replace.execute(lines);
    CHECK_EQ(lines[0], "aa");  // no infinite loop when the replacement contains the pattern
    replace.undo(lines);
    CHECK_EQ(lines[0], "a");
}

TEST_CASE("history supports undo, redo and drops redo after a new edit") {
    Lines lines;
    History history(lines);
    history.run(std::make_unique<AppendLine>("one"));
    history.run(std::make_unique<AppendLine>("two"));
    CHECK(history.undo());
    CHECK(lines == Lines{"one"});
    CHECK(history.redo());
    CHECK(lines == Lines{"one", "two"});
    history.undo();
    history.run(std::make_unique<AppendLine>("three"));
    CHECK(!history.redo());
    CHECK(lines == Lines{"one", "three"});
}

TEST_CASE("a failing command is not recorded") {
    Lines lines{"only"};
    History history(lines);
    CHECK_THROWS_AS(history.run(std::make_unique<DeleteLine>(5)), std::out_of_range);
    CHECK_EQ(history.undo_depth(), 0u);
    CHECK(!history.undo());
}

TEST_CASE("run edits and exports a note") {
    std::istringstream in("add Shopping\nadd buy eggs\nadd buy milk\ndel 2\nundo\nreplace buy get\nexport markdown numbered count\n"
                          "export pdf\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("# 1. Shopping\n- 2. get eggs\n- 3. get milk\n(5 words)") != std::string::npos);
    CHECK(out.str().find("unknown format 'pdf'") != std::string::npos);
}
