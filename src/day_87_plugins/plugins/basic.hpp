// Built-in plugins. Each registers itself; the host never names them in code.
#pragma once

#include <algorithm>

#include "../registry.hpp"

namespace cppm::day87::plugins {

class Invert : public Filter {
public:
    void apply(Image& image) const override {
        for (auto& p : image.pixels) p = static_cast<std::uint8_t>(255 - p);
    }
};

class Threshold : public Filter {
public:
    explicit Threshold(int level) : level_(level) {}
    void apply(Image& image) const override {
        for (auto& p : image.pixels) p = p >= level_ ? 255 : 0;
    }

private:
    int level_;
};

class Brighten : public Filter {
public:
    explicit Brighten(int amount) : amount_(amount) {}
    void apply(Image& image) const override {
        for (auto& p : image.pixels) p = static_cast<std::uint8_t>(std::clamp(p + amount_, 0, 255));
    }

private:
    int amount_;
};

/// 3x3 box blur; edge pixels average only the neighbours that exist.
class BoxBlur : public Filter {
public:
    void apply(Image& image) const override {
        const Image source = image;
        for (int y = 0; y < image.height; ++y) {
            for (int x = 0; x < image.width; ++x) {
                int sum = 0;
                int count = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        const int nx = x + dx;
                        const int ny = y + dy;
                        if (nx < 0 || ny < 0 || nx >= image.width || ny >= image.height) continue;
                        sum += source.at(nx, ny);
                        ++count;
                    }
                }
                image.at(x, y) = static_cast<std::uint8_t>((sum + count / 2) / count);
            }
        }
    }
};

inline const bool invert_registered =
    register_plugin({"invert", "negative image", {2, 0}, [](const Options&) { return std::make_unique<Invert>(); }});
inline const bool threshold_registered = register_plugin({"threshold", "black and white at level=0..255 (default 128)", {2, 1},
                                                          [](const Options& o) {
                                                              return std::make_unique<Threshold>(int_option(o, "level", 128, 0, 255));
                                                          }});
inline const bool brighten_registered = register_plugin({"brighten", "add amount=-255..255 (default 40)", {2, 3}, [](const Options& o) {
                                                             return std::make_unique<Brighten>(int_option(o, "amount", 40, -255, 255));
                                                         }});
inline const bool blur_registered =
    register_plugin({"blur", "3x3 box blur", {2, 2}, [](const Options&) { return std::make_unique<BoxBlur>(); }});

// Two third-party plugins that the host must refuse.
inline const bool legacy_registered =
    register_plugin({"sepia", "written for the old 1.x API", {1, 7}, [](const Options&) { return std::make_unique<Invert>(); }});
inline const bool future_registered =
    register_plugin({"hdr", "needs features from API 2.9", {2, 9}, [](const Options&) { return std::make_unique<Invert>(); }});

}  // namespace cppm::day87::plugins
