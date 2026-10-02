/**
 * @file
 * Day 72 – Design Patterns.
 *
 * Scenario: a *note-taking editor*. Notes are lists of lines that can be exported as plain text,
 * Markdown or HTML (strategy), with exporters created by name from a registry (factory), extra
 * behaviour such as line numbers or a word-count footer stacked on top (decorator), and every
 * edit recorded as an object that can be undone and redone (command).
 *
 * Deliverables (syllabus):
 * - Strategy
 * - Factory
 * - Decorator
 * - Command
 */
#pragma once

#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day72 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"interchangeable export strategies behind one interface", "Exporter"},
    {"a factory registry that creates exporters by name", "make_exporter"},
    {"decorators that add behaviour without subclassing", "WithLineNumbers"},
    {"edits as command objects", "Command"},
    {"undo and redo stacks", "History::undo"},
};

using Lines = std::vector<std::string>;

// ---- Strategy: one interface, several algorithms chosen at run time ----
class Exporter {
  public:
    virtual ~Exporter() = default;
    virtual std::string render(const Lines& lines) const = 0;
};

class PlainExporter : public Exporter {
  public:
    std::string render(const Lines& lines) const override {
        std::string out;
        for (const auto& line : lines) out += line + "\n";
        return out;
    }
};

class MarkdownExporter : public Exporter {
  public:
    std::string render(const Lines& lines) const override {
        std::string out;
        for (std::size_t i = 0; i < lines.size(); ++i) out += (i == 0 ? "# " : "- ") + lines[i] + "\n";
        return out;
    }
};

class HtmlExporter : public Exporter {
  public:
    std::string render(const Lines& lines) const override {
        std::string out = "<ul>\n";
        for (const auto& line : lines) out += "  <li>" + escape(line) + "</li>\n";
        return out + "</ul>\n";
    }

  private:
    static std::string escape(const std::string& text) {
        std::string out;
        for (const char c : text) {
            if (c == '<')
                out += "&lt;";
            else if (c == '>')
                out += "&gt;";
            else if (c == '&')
                out += "&amp;";
            else
                out += c;
        }
        return out;
    }
};

// ---- Factory: callers name what they want; the registry knows how to build it ----
using ExporterFactory = std::function<std::unique_ptr<Exporter>()>;

inline std::map<std::string, ExporterFactory>& exporter_registry() {
    static std::map<std::string, ExporterFactory> registry{
        {"plain", [] { return std::make_unique<PlainExporter>(); }},
        {"markdown", [] { return std::make_unique<MarkdownExporter>(); }},
        {"html", [] { return std::make_unique<HtmlExporter>(); }}};
    return registry;
}

inline std::unique_ptr<Exporter> make_exporter(const std::string& name) {
    const auto it = exporter_registry().find(name);
    if (it == exporter_registry().end()) throw std::invalid_argument("unknown format '" + name + "'");
    return it->second();
}

// ---- Decorator: wraps an Exporter and is itself an Exporter, so decorators stack ----
class WithLineNumbers : public Exporter {
  public:
    explicit WithLineNumbers(std::unique_ptr<Exporter> inner) : inner_(std::move(inner)) {}
    std::string render(const Lines& lines) const override {
        Lines numbered;
        for (std::size_t i = 0; i < lines.size(); ++i) numbered.push_back(std::to_string(i + 1) + ". " + lines[i]);
        return inner_->render(numbered);
    }

  private:
    std::unique_ptr<Exporter> inner_;
};

class WithWordCount : public Exporter {
  public:
    explicit WithWordCount(std::unique_ptr<Exporter> inner) : inner_(std::move(inner)) {}
    std::string render(const Lines& lines) const override {
        std::size_t words = 0;
        for (const auto& line : lines) {
            std::istringstream in(line);
            for (std::string w; in >> w;) ++words;
        }
        return inner_->render(lines) + "(" + std::to_string(words) + " words)\n";
    }

  private:
    std::unique_ptr<Exporter> inner_;
};

// ---- Command: each edit knows how to do and undo itself ----
class Command {
  public:
    virtual ~Command() = default;
    virtual void execute(Lines& lines) = 0;
    virtual void undo(Lines& lines) = 0;
    virtual std::string name() const = 0;
};

class AppendLine : public Command {
  public:
    explicit AppendLine(std::string text) : text_(std::move(text)) {}
    void execute(Lines& lines) override { lines.push_back(text_); }
    void undo(Lines& lines) override { lines.pop_back(); }
    std::string name() const override { return "append"; }

  private:
    std::string text_;
};

class DeleteLine : public Command {
  public:
    explicit DeleteLine(std::size_t index) : index_(index) {}
    void execute(Lines& lines) override {
        if (index_ >= lines.size()) throw std::out_of_range("no line " + std::to_string(index_ + 1));
        removed_ = lines[index_];
        lines.erase(lines.begin() + static_cast<std::ptrdiff_t>(index_));
    }
    void undo(Lines& lines) override { lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(index_), removed_); }
    std::string name() const override { return "delete"; }

  private:
    std::size_t index_;
    std::string removed_;  // remembered so undo can restore it
};

class ReplaceAll : public Command {
  public:
    ReplaceAll(std::string from, std::string to) : from_(std::move(from)), to_(std::move(to)) {}
    void execute(Lines& lines) override {
        before_ = lines;
        if (from_.empty()) return;
        for (auto& line : lines) {
            for (auto at = line.find(from_); at != std::string::npos; at = line.find(from_, at + to_.size())) {
                line.replace(at, from_.size(), to_);
            }
        }
    }
    void undo(Lines& lines) override { lines = before_; }
    std::string name() const override { return "replace"; }

  private:
    std::string from_;
    std::string to_;
    Lines before_;
};

/// Runs commands and keeps undo/redo stacks. A new command clears the redo stack.
class History {
  public:
    explicit History(Lines& lines) : lines_(lines) {}
    void run(std::unique_ptr<Command> command) {
        command->execute(lines_);  // if this throws, nothing is recorded
        done_.push_back(std::move(command));
        undone_.clear();
    }
    bool undo() {
        if (done_.empty()) return false;
        done_.back()->undo(lines_);
        undone_.push_back(std::move(done_.back()));
        done_.pop_back();
        return true;
    }
    bool redo() {
        if (undone_.empty()) return false;
        undone_.back()->execute(lines_);
        done_.push_back(std::move(undone_.back()));
        undone_.pop_back();
        return true;
    }
    std::size_t undo_depth() const { return done_.size(); }

  private:
    Lines& lines_;
    std::vector<std::unique_ptr<Command>> done_;
    std::vector<std::unique_ptr<Command>> undone_;
};

/// The interactive demo: add <text> | del <n> | replace <a> <b> | undo | redo | export <format> [numbered] [count]
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 72 – Design Patterns\n";
    Lines note;
    History history(note);
    while (auto line = prompt_line(in, out, "add|del|replace|undo|redo|export> ")) {
        std::istringstream words(*line);
        std::string command;
        if (!(words >> command)) break;
        try {
            if (command == "add") {
                std::string text;
                std::getline(words >> std::ws, text);
                history.run(std::make_unique<AppendLine>(text));
            } else if (command == "del") {
                std::size_t n = 0;
                words >> n;
                history.run(std::make_unique<DeleteLine>(n - 1));
            } else if (command == "replace") {
                std::string from;
                std::string to;
                words >> from >> to;
                history.run(std::make_unique<ReplaceAll>(from, to));
            } else if (command == "undo") {
                if (!history.undo()) out << "  nothing to undo\n";
            } else if (command == "redo") {
                if (!history.redo()) out << "  nothing to redo\n";
            } else if (command == "export") {
                std::string format;
                words >> format;
                std::unique_ptr<Exporter> exporter = make_exporter(format);
                for (std::string extra; words >> extra;) {
                    if (extra == "numbered") exporter = std::make_unique<WithLineNumbers>(std::move(exporter));
                    if (extra == "count") exporter = std::make_unique<WithWordCount>(std::move(exporter));
                }
                out << exporter->render(note);
            }
        } catch (const std::exception& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day72
