# Day 37 – Graphics Programming Reflection

**Date:** 2026-04-19 · **Level:** Intermediate · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_37_graphics/lesson.hpp`](../../src/day_37_graphics/lesson.hpp) · **Tests:** [`tests/test_day_37.cpp`](../../tests/test_day_37.cpp) (8 tests)

## Scenario
An *event-poster generator* that draws shapes on an in-memory raster canvas – lines, circles, filled areas – and then shows the same picture two ways: as ASCII art in the terminal and as a PPM image file any viewer can open. Drawing never knows how the picture will be displayed, so a window library such as SFML could be added as a third output.

## Syllabus deliverables
> Raster canvas, Bresenham lines, circles and fills, exporting PPM images, separating drawing from display

| Deliverable | Implemented in |
|---|---|
| ✅ a raster canvas with clipped pixel access | `Canvas` |
| ✅ Bresenham's line algorithm with integers only | `Canvas::line` |
| ✅ the midpoint circle algorithm | `Canvas::circle` |
| ✅ flood fill with an explicit stack | `Canvas::flood_fill` |
| ✅ exporting a portable pixmap (PPM) | `to_ppm` |
| ✅ display kept separate from drawing | `to_ascii` |

## Key learnings
- A raster canvas is just a width, a height and a vector of colours indexed by `y * width + x`.
- Bresenham's line and the midpoint circle algorithms decide every pixel with integer additions only, which is why they were ideal for early hardware.
- An iterative flood fill with an explicit stack handles large areas that would overflow the call stack with recursion.
- Keeping drawing separate from display means the same picture can become ASCII art, a PPM file or, later, an SFML window.

## Pitfalls I hit (and how I fixed them)
- Drawing a circle that crossed the canvas edge wrote outside the vector; `set` now clips pixels outside the canvas.
- A recursive flood fill crashed on a 2000×2000 canvas; the explicit stack fixed it.
- Printing `uint8_t` colour values into the PPM wrote raw bytes; unary `+` promotes them to numbers.

## Run it
```bash
./procpp.sh 37                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_37_graphics       # the interactive demo
ctest --test-dir build -R test_day_37 --output-on-failure
```

## Next step
- Day 38 puts objects together into a small game with a loop and rules.
