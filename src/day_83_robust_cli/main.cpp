#include <iostream>

#include "lesson.hpp"

// With arguments it is the real tool (`day_83_robust_cli --json list`); without, the interactive demo.
int main(int argc, char** argv) {
    if (argc > 1) return cppm::day83::run(cppm::day83::arguments_from(argc, argv), std::cout, std::cerr);
    return cppm::day83::run(std::cin, std::cout);
}
