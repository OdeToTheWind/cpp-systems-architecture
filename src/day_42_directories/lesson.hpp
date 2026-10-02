/**
 * @file
 * Day 42 – Working with Directories.
 *
 * Scenario: a *photo-shoot ingest tool*. After a shoot, a memory card is dumped into an
 * inbox folder; the tool walks it, plans where every file belongs (raw/, jpeg/, video/,
 * sidecar/, other/), avoids name collisions, prints the resulting tree – and refuses to touch
 * anything outside the one folder it was given, so a typo can never damage the rest of the disk.
 *
 * Deliverables (syllabus):
 * - std::filesystem paths
 * - Creating and walking directories
 * - Filtering by extension
 * - Sandboxed operations
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day42 {

namespace fs = std::filesystem;

inline constexpr Deliverable DELIVERABLES[] = {
    {"a sandbox that rejects paths escaping its root", "Sandbox::resolve"},
    {"classifying files by extension", "category_for"},
    {"walking a directory tree recursively", "find_files"},
    {"planning moves with collision-free names", "plan_ingest"},
    {"creating folders and moving files", "apply_plan"},
    {"printing a directory tree", "tree"},
};

/// All operations happen inside one root folder; anything that would leave it is refused.
class Sandbox {
  public:
    explicit Sandbox(const fs::path& root) : root_(fs::weakly_canonical(root)) {
        if (!fs::is_directory(root_)) {
            throw std::invalid_argument("sandbox root must be an existing directory");
        }
    }
    const fs::path& root() const { return root_; }

    /// A path inside the sandbox, or std::invalid_argument for absolute paths and ".." escapes.
    fs::path resolve(const fs::path& relative) const {
        if (relative.is_absolute()) {
            throw std::invalid_argument("absolute paths are not allowed: " + relative.string());
        }
        const fs::path candidate = (root_ / relative).lexically_normal();
        const fs::path inside = candidate.lexically_relative(root_);
        if (inside.empty() || *inside.begin() == "..") {
            throw std::invalid_argument("path escapes the sandbox: " + relative.string());
        }
        return candidate;
    }

  private:
    fs::path root_;
};

inline std::string lower_extension(const fs::path& file) {
    std::string extension = file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension;
}

/// raw, jpeg, video, sidecar or other – decided by the (case-insensitive) extension.
inline std::string category_for(const fs::path& file) {
    static const std::map<std::string, std::string> categories{
        {".cr2", "raw"},   {".nef", "raw"},   {".arw", "raw"},   {".dng", "raw"},     {".jpg", "jpeg"},
        {".jpeg", "jpeg"}, {".mp4", "video"}, {".mov", "video"}, {".xmp", "sidecar"},
    };
    const auto found = categories.find(lower_extension(file));
    return found == categories.end() ? "other" : found->second;
}

/// Every regular file under @p folder (recursively), optionally only one category, sorted.
inline std::vector<fs::path> find_files(const fs::path& folder, const std::string& only_category = "") {
    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(folder)) {
        if (entry.is_regular_file() && (only_category.empty() || category_for(entry.path()) == only_category)) {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

/// One planned move.
struct Move {
    fs::path from;
    fs::path to;
};

/// Decide a destination for every file in inbox/; "IMG_1.jpg" becomes "IMG_1_1.jpg" if the name is taken.
inline std::vector<Move> plan_ingest(const Sandbox& sandbox, const fs::path& inbox = "inbox",
                                     const fs::path& library = "library") {
    const fs::path source = sandbox.resolve(inbox);
    const fs::path target = sandbox.resolve(library);
    std::vector<Move> plan;
    std::vector<fs::path> taken;
    for (const auto& file : find_files(source)) {
        fs::path destination = target / category_for(file) / file.filename();
        for (int suffix = 1;
             fs::exists(destination) || std::find(taken.begin(), taken.end(), destination) != taken.end(); ++suffix) {
            destination = target / category_for(file) /
                          (file.stem().string() + "_" + std::to_string(suffix) + file.extension().string());
        }
        taken.push_back(destination);
        plan.push_back({file, destination});
    }
    return plan;
}

/// Execute a plan: create folders as needed and move each file. Returns the number moved.
inline int apply_plan(const Sandbox& sandbox, const std::vector<Move>& plan) {
    int moved = 0;
    for (const auto& move : plan) {
        // re-check every path against the sandbox: a plan could have been edited or built elsewhere
        const auto from = sandbox.resolve(move.from.lexically_relative(sandbox.root()));
        const auto to = sandbox.resolve(move.to.lexically_relative(sandbox.root()));
        fs::create_directories(to.parent_path());
        fs::rename(from, to);
        ++moved;
    }
    return moved;
}

/// An indented listing of @p folder: directories end with '/', entries sorted at each level.
inline std::string tree(const fs::path& folder, int depth = 0) {
    std::vector<fs::directory_entry> entries(fs::directory_iterator(folder), fs::directory_iterator{});
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.path().filename() < b.path().filename(); });
    std::string out;
    for (const auto& entry : entries) {
        out += std::string(static_cast<std::size_t>(depth) * 2, ' ') + entry.path().filename().string();
        if (entry.is_directory()) {
            out += "/\n" + tree(entry.path(), depth + 1);
        } else {
            out += '\n';
        }
    }
    return out;
}

/// The interactive demo: works in @p root (a fresh sandbox), creating a sample card dump first.
inline int run(std::istream& in, std::ostream& out, const fs::path& root = fs::temp_directory_path() / "cppm-day42") {
    out << "Day 42 – Working with Directories\nSandbox: " << root.string() << '\n';
    fs::create_directories(root / "inbox" / "DCIM" / "100CANON");
    for (const char* name :
         {"IMG_0001.CR2", "IMG_0001.JPG", "IMG_0002.jpg", "MVI_0003.MOV", "IMG_0001.xmp", "notes.txt"}) {
        std::ofstream(root / "inbox" / "DCIM" / "100CANON" / name) << name;
    }
    const Sandbox sandbox(root);
    const auto plan = plan_ingest(sandbox);
    for (const auto& move : plan) {
        out << "  " << move.from.filename().string() << " -> " << move.to.lexically_relative(root).generic_string()
            << '\n';
    }
    auto answer = prompt_line(in, out, "Apply this plan? (y/n): ");
    if (answer && *answer == "y") {
        out << apply_plan(sandbox, plan) << " file(s) moved\n" << tree(root / "library");
    } else {
        out << "Dry run only – nothing was moved.\n";
    }
    return 0;
}

}  // namespace cppm::day42
