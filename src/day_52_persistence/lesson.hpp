/**
 * @file
 * Day 52 – Local Persistence.
 *
 * Scenario: a *houseplant care app* that remembers every plant's watering interval and the day
 * it was last watered between runs. Saving must never leave a half-written file behind, old
 * save files from version 1 of the app must still load, and a corrupt file is set aside instead
 * of crashing the app or silently losing it.
 *
 * Deliverables (syllabus):
 * - Saving and loading application state
 * - Atomic writes
 * - Schema versions
 * - Recovering from corrupt files
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day52 {

namespace fs = std::filesystem;

inline constexpr Deliverable DELIVERABLES[] = {
    {"the application state to persist", "GardenState"},
    {"serialising with a version header", "serialise"},
    {"parsing every supported schema version", "deserialise"},
    {"atomic save: write a temp file, then rename", "save_atomically"},
    {"loading with recovery from corrupt files", "load_or_recover"},
};

inline constexpr int current_version = 2;

struct Plant {
    int interval_days{7};
    int last_watered_day{0};
    bool operator==(const Plant&) const = default;
};

struct GardenState {
    std::map<std::string, Plant> plants;
    bool operator==(const GardenState&) const = default;

    std::vector<std::string> due(int today) const {
        std::vector<std::string> names;
        for (const auto& [name, plant] : plants) {
            if (today - plant.last_watered_day >= plant.interval_days) names.push_back(name);
        }
        return names;
    }
};

/// "version 2" then one "plant <name> <interval> <last>" line per plant. Names may not contain spaces.
inline std::string serialise(const GardenState& state) {
    std::ostringstream out;
    out << "version " << current_version << '\n';
    for (const auto& [name, plant] : state.plants) {
        out << "plant " << name << ' ' << plant.interval_days << ' ' << plant.last_watered_day << '\n';
    }
    return out.str();
}

/// Accepts version 1 ("plant <name> <last>", interval defaulted to 7) and version 2.
/// Throws std::runtime_error for anything it does not understand.
inline GardenState deserialise(const std::string& text) {
    std::istringstream in(text);
    std::string word;
    int version = 0;
    if (!(in >> word >> version) || word != "version") throw std::runtime_error("missing version header");
    if (version < 1 || version > current_version) {
        throw std::runtime_error("unsupported save version " + std::to_string(version));
    }
    GardenState state;
    std::string line;
    std::getline(in, line);  // rest of the header line
    int number = 1;
    while (std::getline(in, line)) {
        ++number;
        if (line.empty()) continue;
        std::istringstream fields(line);
        std::string tag;
        std::string name;
        Plant plant;
        const bool ok = version == 1 ? static_cast<bool>(fields >> tag >> name >> plant.last_watered_day)
                                     : static_cast<bool>(fields >> tag >> name >> plant.interval_days >> plant.last_watered_day);
        if (!ok || tag != "plant" || plant.interval_days <= 0 || !(fields >> std::ws).eof()) {
            throw std::runtime_error("corrupt line " + std::to_string(number));
        }
        state.plants[name] = plant;
    }
    return state;
}

/// Write to "<file>.tmp", flush, then rename over the real file. A crash at any moment leaves
/// either the complete old file or the complete new one – never half of each.
inline void save_atomically(const fs::path& file, const GardenState& state) {
    const fs::path temporary = file.string() + ".tmp";
    {
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("cannot write " + temporary.string());
        out << serialise(state);
        out.flush();
        if (!out) throw std::runtime_error("write failed for " + temporary.string());
    }  // closed before the rename
    fs::rename(temporary, file);  // atomic replace on POSIX; replaces the target on Windows too
}

/// What loading produced.
struct LoadResult {
    GardenState state;
    bool recovered{false};    // the file was corrupt and has been moved aside
    fs::path quarantined;     // where the corrupt file went
};

/// Missing file -> empty garden. Corrupt file -> renamed to "<file>.corrupt", empty garden, recovered = true.
inline LoadResult load_or_recover(const fs::path& file) {
    if (!fs::exists(file)) return {};
    std::ifstream in(file, std::ios::binary);
    std::ostringstream text;
    text << in.rdbuf();
    try {
        return {deserialise(text.str()), false, {}};
    } catch (const std::runtime_error&) {
        const fs::path aside = file.string() + ".corrupt";
        in.close();
        fs::rename(file, aside);  // keep it for inspection – never delete user data
        return {GardenState{}, true, aside};
    }
}

/// The interactive demo: add/water/due commands; the garden is saved after every change.
inline int run(std::istream& in, std::ostream& out, const fs::path& file = fs::temp_directory_path() / "cppm-garden.txt") {
    out << "Day 52 – Local Persistence\nSave file: " << file.string() << '\n';
    auto [state, recovered, aside] = load_or_recover(file);
    if (recovered) out << "The save file was damaged; it was kept as " << aside.filename().string() << '\n';
    out << state.plants.size() << " plant(s) loaded\n";
    while (auto line = prompt_line(in, out, "add <name> <days> | water <name> <day> | due <day>> ")) {
        std::istringstream words(*line);
        std::string command;
        std::string name;
        if (!(words >> command)) break;
        if (command == "due") {
            int today = 0;
            words >> today;
            for (const auto& plant : state.due(today)) out << "  water " << plant << '\n';
            continue;
        }
        int number = 0;
        if (!(words >> name >> number)) {
            out << "  missing arguments\n";
            continue;
        }
        if (command == "add") state.plants[name] = Plant{number, 0};
        else if (command == "water" && state.plants.contains(name)) state.plants[name].last_watered_day = number;
        else out << "  unknown command or plant\n";
        save_atomically(file, state);
    }
    return 0;
}

}  // namespace cppm::day52
