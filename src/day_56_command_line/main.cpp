#include <iostream>

#include "lesson.hpp"

// With arguments it behaves like a real tool (`day_56_command_line -n ERROR app.log`);
// without arguments it starts the interactive demo.
int main(int argc, char** argv) {
    if (argc > 1) {
        return cppm::day56::run(cppm::day56::arguments_from(argc, argv), std::cin, std::cout, std::cerr);
    }
    return cppm::day56::run(std::cin, std::cout);
}
