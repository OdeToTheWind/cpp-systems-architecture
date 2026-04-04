#include <iostream>
#include <string>

// Pass-by-value (copy is made)
void swap_by_value(int a, int b) {
    int temp = a;
    a = b;
    b = temp;
}

// Pass-by-reference (modifies original)
void swap_by_reference(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

// Pass-by-pointer (can be null)
void swap_by_pointer(int* a, int* b) {
    if (a == nullptr || b == nullptr) {
        std::cout << "Error: Null pointer passed!\n";
        return;
    }
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function that modifies value through reference
void increment_by_reference(int& num, int amount) {
    num += amount;
}

// Function that modifies value through pointer
void increment_by_pointer(int* num, int amount) {
    if (num) {
        *num += amount;
    }
}

int main() {
    std::cout << "=== Day 19: Pointers and References ===\n\n";

    int x, y;
    std::cout << "Enter two integers (x y): ";
    std::cin >> x >> y;

    std::cout << "Original: x = " << x << ", y = " << y << "\n\n";

    // 1. Pass-by-value demonstration
    swap_by_value(x, y);
    std::cout << "After swap_by_value (no change): x = " << x << ", y = " << y << "\n";

    // 2. Pass-by-reference (recommended)
    swap_by_reference(x, y);
    std::cout << "After swap_by_reference: x = " << x << ", y = " << y << "\n\n";

    // 3. Pass-by-pointer
    int a = 100, b = 200;
    std::cout << "Pointer example - a = " << a << ", b = " << b << "\n";
    swap_by_pointer(&a, &b);
    std::cout << "After swap_by_pointer: a = " << a << ", b = " << b << "\n\n";

    // 4. Modifying through reference vs pointer
    int value = 50;
    std::cout << "Before increment: value = " << value << "\n";

    increment_by_reference(value, 25);
    std::cout << "After increment_by_reference: value = " << value << "\n";

    increment_by_pointer(&value, 15);
    std::cout << "After increment_by_pointer: value = " << value << "\n\n";

    // 5. Address demonstration
    std::cout << "Memory addresses:\n";
    std::cout << "&x = " << &x << "\n";
    std::cout << "&y = " << &y << "\n";
    std::cout << "&value = " << &value << "\n";

    std::cout << "\nKey takeaway:\n"
              << "  * References (`&`) are safer and cleaner for most cases\n"
              << "  * Pointers (`*`) give more control but require careful handling\n"
              << "  * Prefer `const T&` when you only need to read large objects\n"
              << "  * Always initialize pointers (use `nullptr`)\n";

    return 0;
}