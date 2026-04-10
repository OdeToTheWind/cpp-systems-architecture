#include <iostream>

int main() {
    std::cout << "=== Day 26: IDE Tips and Tricks ===\n\n";

    std::cout << "Here are some powerful IDE features with practical examples:\n\n";

    std::cout << "1. Go to Definition / Declaration\n";
    std::cout << "   -- Ctrl + Click on any function or variable to jump to its definition.\n";
    std::cout << "   Example: Click on 'calculate_bmi' to see its implementation instantly.\n\n";

    std::cout << "2. Find All References\n";
    std::cout << "   -- Right-click on a variable or function -- 'Find All References'.\n";
    std::cout << "   Shows everywhere that variable/function is used in the project.\n\n";

    std::cout << "3. Rename Symbol (Refactoring)\n";
    std::cout << "   -- Right-click on a variable -- 'Rename Symbol'.\n";
    std::cout << "   Safely renames it everywhere in the project without missing any occurrence.\n\n";

    std::cout << "4. Integrated Debugger\n";
    std::cout << "   -- Set breakpoints by clicking in the gutter.\n";
    std::cout << "   -- Step through code line by line, inspect variable values while running.\n\n";

    std::cout << "5. CMake Tools Integration\n";
    std::cout << "   -- Build and run directly from the IDE using CMake configuration.\n";
    std::cout << "   No need to manually run './procpp.sh' every time.\n\n";

    std::cout << "6. Code Formatting (clang-format)\n";
    std::cout << "   -- Automatically formats your code to consistent style (indentation, spacing).\n\n";

    std::cout << "7. IntelliSense / Code Completion\n";
    std::cout << "   -- As you type, the IDE suggests functions, variables, and parameters.\n";

    std::cout << "\nKey takeaway:\n";
    std::cout << "* Mastering these IDE features significantly increases your productivity.\n";
    std::cout << "* A good IDE turns C++ development from painful to enjoyable.\n";

    return 0;
}