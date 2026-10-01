/**
 * @file
 * Day 37 – Graphics Programming.
 *
 * Scenario: an *event-poster generator* that draws shapes on an in-memory raster canvas –
 * lines, circles, filled areas – and then shows the same picture two ways: as ASCII art in
 * the terminal and as a PPM image file any viewer can open. Drawing never knows how the
 * picture will be displayed, so a window library such as SFML could be added as a third output.
 *
 * Deliverables (syllabus):
 * - Raster canvas
 * - Bresenham lines, circles and fills
 * - Exporting PPM images
 * - Separating drawing from display
 */
#pragma once

#include <cstdint>
#include <cstdlib>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day37 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a raster canvas with clipped pixel access", "Canvas"},
    {"Bresenham's line algorithm with integers only", "Canvas::line"},
    {"the midpoint circle algorithm", "Canvas::circle"},
    {"flood fill with an explicit stack", "Canvas::flood_fill"},
    {"exporting a portable pixmap (PPM)", "to_ppm"},
    {"display kept separate from drawing", "to_ascii"},
};

struct Color {
    std::uint8_t r{255};
    std::uint8_t g{255};
    std::uint8_t b{255};
    bool operator==(const Color&) const = default;
};

inline constexpr Color white{255, 255, 255};
inline constexpr Color black{0, 0, 0};
inline constexpr Color red{220, 30, 40};
inline constexpr Color gold{240, 190, 40};

class Canvas {
public:
    Canvas(int width, int height, Color background = white) : width_(width), height_(height) {
        if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
            throw std::invalid_argument("canvas size must be 1-4096 pixels per side");
        }
        pixels_.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), background);
    }
    int width() const { return width_; }
    int height() const { return height_; }
    bool inside(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }

    /// Pixels outside the canvas are clipped (ignored), so shapes may cross the border safely.
    void set(int x, int y, Color color) {
        if (inside(x, y)) {
            pixels_[index(x, y)] = color;
        }
    }
    Color get(int x, int y) const {
        if (!inside(x, y)) {
            throw std::out_of_range("pixel outside the canvas");
        }
        return pixels_[index(x, y)];
    }

    /// Bresenham: steps one pixel at a time along the major axis, using only integer additions.
    void line(int x0, int y0, int x1, int y1, Color color) {
        const int dx = std::abs(x1 - x0);
        const int dy = -std::abs(y1 - y0);
        const int step_x = x0 < x1 ? 1 : -1;
        const int step_y = y0 < y1 ? 1 : -1;
        int error = dx + dy;
        while (true) {
            set(x0, y0, color);
            if (x0 == x1 && y0 == y1) {
                break;
            }
            const int doubled = 2 * error;
            if (doubled >= dy) {
                error += dy;
                x0 += step_x;
            }
            if (doubled <= dx) {
                error += dx;
                y0 += step_y;
            }
        }
    }

    /// Midpoint circle: compute one octant and mirror it eight ways.
    void circle(int cx, int cy, int radius, Color color) {
        if (radius < 0) {
            throw std::invalid_argument("radius must not be negative");
        }
        int x = radius;
        int y = 0;
        int decision = 1 - radius;
        while (x >= y) {
            for (const auto& [px, py] : {std::pair{x, y}, {y, x}, {-y, x}, {-x, y}, {-x, -y}, {-y, -x}, {y, -x}, {x, -y}}) {
                set(cx + px, cy + py, color);
            }
            ++y;
            if (decision < 0) {
                decision += 2 * y + 1;
            } else {
                --x;
                decision += 2 * (y - x) + 1;
            }
        }
    }

    void fill_rect(int x, int y, int w, int h, Color color) {
        for (int row = y; row < y + h; ++row) {
            for (int col = x; col < x + w; ++col) {
                set(col, row, color);
            }
        }
    }

    /// Replace the connected area of the start pixel's colour. An explicit stack instead of
    /// recursion: a large area would otherwise overflow the call stack. Returns pixels changed.
    int flood_fill(int x, int y, Color color) {
        if (!inside(x, y)) {
            return 0;
        }
        const Color target = get(x, y);
        if (target == color) {
            return 0;
        }
        int changed = 0;
        std::vector<std::pair<int, int>> stack{{x, y}};
        while (!stack.empty()) {
            const auto [px, py] = stack.back();
            stack.pop_back();
            if (!inside(px, py) || get(px, py) != target) {
                continue;
            }
            set(px, py, color);
            ++changed;
            stack.insert(stack.end(), {{px + 1, py}, {px - 1, py}, {px, py + 1}, {px, py - 1}});
        }
        return changed;
    }

    int count(Color color) const {
        int n = 0;
        for (const auto& pixel : pixels_) {
            n += pixel == color ? 1 : 0;
        }
        return n;
    }

private:
    std::size_t index(int x, int y) const {
        return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);
    }
    int width_;
    int height_;
    std::vector<Color> pixels_;
};

/// Plain-text PPM ("P3"): a header, then one "r g b" triple per pixel.
inline std::string to_ppm(const Canvas& canvas) {
    std::ostringstream out;
    out << "P3\n" << canvas.width() << ' ' << canvas.height() << "\n255\n";
    for (int y = 0; y < canvas.height(); ++y) {
        for (int x = 0; x < canvas.width(); ++x) {
            const Color c = canvas.get(x, y);
            out << +c.r << ' ' << +c.g << ' ' << +c.b << (x + 1 == canvas.width() ? '\n' : ' ');
        }
    }
    return out.str();
}

/// A terminal preview: light pixels become spaces, darker ones '+', '*' or '#'.
inline std::string to_ascii(const Canvas& canvas) {
    std::string out;
    for (int y = 0; y < canvas.height(); ++y) {
        for (int x = 0; x < canvas.width(); ++x) {
            const Color c = canvas.get(x, y);
            const int brightness = (c.r * 3 + c.g * 6 + c.b) / 10;
            out += brightness > 230 ? ' ' : brightness > 150 ? '+' : brightness > 70 ? '*' : '#';
        }
        out += '\n';
    }
    return out;
}

/// The poster used by the demo: a frame, a sun, its rays and a filled sun.
inline Canvas draw_poster(int size) {
    Canvas canvas(size, size);
    canvas.line(0, 0, size - 1, 0, black);
    canvas.line(0, size - 1, size - 1, size - 1, black);
    canvas.line(0, 0, 0, size - 1, black);
    canvas.line(size - 1, 0, size - 1, size - 1, black);
    const int centre = size / 2;
    canvas.circle(centre, centre, size / 4, red);
    canvas.flood_fill(centre, centre, gold);
    canvas.line(centre, centre - size / 4 - 1, centre, 2, red);
    return canvas;
}

/// The interactive demo: choose a poster size; it is shown as ASCII and as PPM header lines.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 37 – Graphics Programming\n";
    auto line = prompt_line(in, out, "Poster size in pixels (8-60): ");
    int size = 20;
    if (line && !(std::istringstream(*line) >> size)) {
        size = 20;
    }
    if (size < 8 || size > 60) {
        out << "  using 20 instead\n";
        size = 20;
    }
    const Canvas poster = draw_poster(size);
    out << to_ascii(poster);
    const std::string ppm = to_ppm(poster);
    out << "PPM starts with: " << ppm.substr(0, ppm.find('\n', 3)) << " (" << ppm.size() << " bytes)\n";
    return 0;
}

}  // namespace cppm::day37
