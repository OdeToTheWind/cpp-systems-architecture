/**
 * @file
 * Day 67 – Smart Pointers & Ownership.
 *
 * Scenario: the object model of a *slide-deck editor*. A deck owns its slides outright, slides
 * share image assets that are freed when no slide uses them, and grouped shapes point back to
 * their parent group without keeping it alive. Every object counts itself, so tests can prove
 * that nothing leaks and nothing is freed too early.
 *
 * Deliverables (syllabus):
 * - unique_ptr, shared_ptr and weak_ptr
 * - Ownership transfer
 * - Breaking reference cycles
 */
#pragma once

#include <algorithm>
#include <cstddef>
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

namespace cppm::day67 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"exclusive ownership with unique_ptr", "Deck::add_slide"},
    {"transferring ownership out of a container", "Deck::take_slide"},
    {"shared assets with a weak_ptr cache", "AssetCache::load"},
    {"weak back-references that break cycles", "Shape::path"},
    {"what a shared_ptr cycle does to lifetimes", "cycle_survives_scope"},
};

/// Counts live instances of each derived type, so tests can check for leaks.
template <typename T>
struct Counted {
    static inline int live = 0;
    Counted() { ++live; }
    Counted(const Counted&) { ++live; }
    Counted& operator=(const Counted&) = default;
    ~Counted() { --live; }
};

struct Image : Counted<Image> {
    explicit Image(std::string file) : path(std::move(file)) {}
    std::string path;
};

/// Hands out shared images; holds only weak_ptrs, so the cache never keeps an image alive.
class AssetCache {
  public:
    std::shared_ptr<Image> load(const std::string& path) {
        if (auto cached = cache_[path].lock()) {
            ++hits_;
            return cached;
        }
        auto image = std::make_shared<Image>(path);  // one allocation for object and control block
        cache_[path] = image;
        ++loads_;
        return image;
    }
    int loads() const { return loads_; }
    int hits() const { return hits_; }

  private:
    std::map<std::string, std::weak_ptr<Image>> cache_;
    int loads_ = 0;
    int hits_ = 0;
};

struct Slide : Counted<Slide> {
    explicit Slide(std::string name) : title(std::move(name)) {}
    std::string title;
    std::vector<std::shared_ptr<Image>> images;  // shared with other slides
};

/// The deck owns its slides: when the deck goes, they go. Callers get plain pointers or
/// references for *viewing*, which never imply ownership.
class Deck {
  public:
    // Spelled out: std::is_copy_constructible says "yes" for vector<unique_ptr<T>> even though
    // the copy would not compile, so deleting the copy makes the move-only intent checkable.
    Deck() = default;
    Deck(const Deck&) = delete;
    Deck& operator=(const Deck&) = delete;
    Deck(Deck&&) = default;
    Deck& operator=(Deck&&) = default;
    ~Deck() = default;

    Slide& add_slide(const std::string& title) {
        slides_.push_back(std::make_unique<Slide>(title));
        return *slides_.back();
    }
    /// Move a slide out of the deck: the caller becomes the owner (e.g. for an undo stack).
    std::unique_ptr<Slide> take_slide(std::size_t index) {
        if (index >= slides_.size()) throw std::out_of_range("no slide " + std::to_string(index));
        auto slide = std::move(slides_[index]);
        slides_.erase(slides_.begin() + static_cast<std::ptrdiff_t>(index));
        return slide;
    }
    /// Take ownership of a slide created elsewhere, e.g. restored by undo.
    void insert_slide(std::size_t index, std::unique_ptr<Slide> slide) {
        if (!slide) throw std::invalid_argument("cannot insert a null slide");
        slides_.insert(slides_.begin() + static_cast<std::ptrdiff_t>(std::min(index, slides_.size())),
                       std::move(slide));
    }
    const Slide* find(const std::string& title) const {
        for (const auto& s : slides_) {
            if (s->title == title) return s.get();
        }
        return nullptr;
    }
    std::size_t size() const { return slides_.size(); }

  private:
    std::vector<std::unique_ptr<Slide>> slides_;
};

/// A shape in a tree of groups. Children are owned (shared_ptr); the parent link is weak,
/// otherwise parent and child would keep each other alive forever.
class Shape : public Counted<Shape>, public std::enable_shared_from_this<Shape> {
  public:
    explicit Shape(std::string name) : name_(std::move(name)) {}
    std::shared_ptr<Shape> add_child(const std::string& name) {
        auto child = std::make_shared<Shape>(name);
        child->parent_ = weak_from_this();
        child->has_parent_ = true;
        children_.push_back(child);
        return child;
    }
    /// "group/subgroup/shape"; a parent that no longer exists is shown as "?".
    std::string path() const {
        if (!has_parent_) return name_;
        if (const auto parent = parent_.lock()) return parent->path() + "/" + name_;
        return "?/" + name_;
    }
    std::size_t child_count() const { return children_.size(); }

  private:
    std::string name_;
    std::weak_ptr<Shape> parent_;
    bool has_parent_ = false;
    std::vector<std::shared_ptr<Shape>> children_;
};

/// Two nodes that hold *strong* pointers to each other. Returns whether they are still alive
/// after the last outside shared_ptr is gone (they are: the cycle keeps both counts at 1),
/// then breaks the cycle so nothing actually leaks.
struct StrongNode : Counted<StrongNode> {
    std::shared_ptr<StrongNode> other;
};

inline bool cycle_survives_scope() {
    std::weak_ptr<StrongNode> observer;
    {
        auto a = std::make_shared<StrongNode>();
        auto b = std::make_shared<StrongNode>();
        a->other = b;
        b->other = a;
        observer = a;
    }  // a and b go out of scope, but each node still owns the other
    const bool survived = !observer.expired();
    if (auto a = observer.lock()) {
        auto b = a->other;
        b->other.reset();  // break the cycle by hand
        a->other.reset();
    }
    return survived;
}

/// The interactive demo: "add <title> <image>", "take <index>", "undo", "list".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 67 – Smart Pointers & Ownership\n";
    AssetCache assets;
    std::vector<std::unique_ptr<Slide>> undo;
    {
        Deck deck;
        while (auto line = prompt_line(in, out, "add <title> <image> | take <i> | undo> ")) {
            std::istringstream words(*line);
            std::string command;
            if (!(words >> command)) break;
            try {
                if (command == "add") {
                    std::string title;
                    std::string image;
                    words >> title >> image;
                    deck.add_slide(title).images.push_back(assets.load(image));
                } else if (command == "take") {
                    std::size_t index = 0;
                    words >> index;
                    undo.push_back(deck.take_slide(index));
                    out << "  removed '" << undo.back()->title << "' (kept for undo)\n";
                } else if (command == "undo" && !undo.empty()) {
                    deck.insert_slide(deck.size(), std::move(undo.back()));
                    undo.pop_back();
                }
            } catch (const std::exception& error) {
                out << "  " << error.what() << '\n';
            }
            out << "  slides " << deck.size() << ", images in memory " << Image::live << '\n';
        }
        out << "assets: " << assets.loads() << " load(s), " << assets.hits() << " cache hit(s)\n";
    }
    undo.clear();
    out << "after closing: " << Slide::live << " slide(s), " << Image::live << " image(s) alive\n";
    return 0;
}

}  // namespace cppm::day67
