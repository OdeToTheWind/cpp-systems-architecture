// The plugin API of the Day 87 image tool: everything a plugin may depend on, and nothing else.
#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace cppm::day87 {

/// The host's API version. Plugins state the version they were written against.
struct ApiVersion {
    int major;
    int minor;
};
inline constexpr ApiVersion HOST_API{2, 3};

/// A grayscale image, one byte per pixel, row by row.
struct Image {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels;
    std::uint8_t& at(int x, int y) { return pixels[static_cast<std::size_t>(y * width + x)]; }
    std::uint8_t at(int x, int y) const { return pixels[static_cast<std::size_t>(y * width + x)]; }
};

using Options = std::map<std::string, std::string>;

/// The interface every filter plugin implements.
class Filter {
public:
    virtual ~Filter() = default;
    virtual void apply(Image& image) const = 0;
};

/// What a plugin tells the host about itself, plus a factory for configured instances.
struct PluginInfo {
    std::string name;
    std::string description;
    ApiVersion requires_api;
    std::function<std::unique_ptr<Filter>(const Options&)> create;
};

/// Read an integer option in [low, high], with a default when absent.
inline int int_option(const Options& options, const std::string& key, int fallback, int low, int high) {
    const auto it = options.find(key);
    if (it == options.end()) return fallback;
    std::size_t used = 0;
    int value = 0;
    try {
        value = std::stoi(it->second, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != it->second.size() || value < low || value > high) {
        throw std::invalid_argument(key + " must be an integer from " + std::to_string(low) + " to " + std::to_string(high));
    }
    return value;
}

}  // namespace cppm::day87
