/**
 * @file
 * Day 87 – Capstone: Plugin Architecture.
 *
 * Scenario: a *photo-filter tool* whose filters come from plugins. The host knows only the plugin
 * API (plugin_api.hpp): plugins register themselves with a registry, the host creates configured
 * instances through their factories by name, and plugins written for an incompatible API version
 * are refused with a reason instead of crashing at run time.
 *
 * Deliverables (syllabus):
 * - Plugin interfaces
 * - Self-registration
 * - Factories
 * - Versioned compatibility
 */
#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"
#include "plugin_api.hpp"
#include "plugins/basic.hpp"
#include "registry.hpp"

namespace cppm::day87 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"the interface every plugin implements", "Filter"},
    {"plugin metadata with a factory and required API", "PluginInfo"},
    {"version checks when a plugin registers", "PluginRegistry::add"},
    {"self-registration from a plugin header", "register_plugin"},
    {"building a pipeline from plugin names and options", "build_pipeline"},
};

/// "invert | threshold level=100 | blur" -> configured filters, created through the registry.
inline std::vector<std::unique_ptr<Filter>> build_pipeline(const std::string& spec) {
    std::vector<std::unique_ptr<Filter>> filters;
    std::istringstream stages(spec);
    for (std::string stage; std::getline(stages, stage, '|');) {
        std::istringstream words(stage);
        std::string name;
        if (!(words >> name)) continue;
        Options options;
        for (std::string kv; words >> kv;) {
            const auto eq = kv.find('=');
            if (eq == std::string::npos) throw std::invalid_argument("option '" + kv + "' needs key=value");
            options[kv.substr(0, eq)] = kv.substr(eq + 1);
        }
        filters.push_back(PluginRegistry::instance().create(name, options));
    }
    return filters;
}

inline Image gradient(int width, int height) {
    Image image{width, height, std::vector<std::uint8_t>(static_cast<std::size_t>(width * height))};
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) image.at(x, y) = static_cast<std::uint8_t>((x * 255) / (width - 1));
    }
    return image;
}

inline std::string ascii(const Image& image) {
    static const std::string shades = " .:-=+*#%@";
    std::string out;
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) out += shades[static_cast<std::size_t>(image.at(x, y) * 9 / 255)];
        out += '\n';
    }
    return out;
}

/// The interactive demo: type a pipeline such as "invert | threshold level=100".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 87 – Capstone: Plugin Architecture\nhost API " << HOST_API.major << '.' << HOST_API.minor
        << "; plugins:\n";
    for (const PluginInfo* p : PluginRegistry::instance().list()) {
        out << "  " << p->name << " (API " << p->requires_api.major << '.' << p->requires_api.minor << ") – "
            << p->description << '\n';
    }
    for (const auto& [name, why] : PluginRegistry::instance().rejected())
        out << "  rejected " << name << ": " << why << '\n';
    while (auto line = prompt_line(in, out, "pipeline> ")) {
        if (line->empty()) break;
        try {
            Image image = gradient(20, 2);
            for (const auto& filter : build_pipeline(*line)) filter->apply(image);
            out << ascii(image);
        } catch (const std::exception& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day87
