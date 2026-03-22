#include <iostream>
#include <iomanip>
#include <cstdint>      // fixed-width types
#include <limits>       // numeric_limits

int main() {
    std::cout << "=== Day 07: Type Conversion & Casting ===\n\n";

    // ── Input values from user ─────────────────────────────────────────────────
    double decimal_value;
    std::cout << "Enter a decimal number (e.g. 42.75): ";
    std::cin >> decimal_value;

    int integer_input;
    std::cout << "Enter an integer: ";
    std::cin >> integer_input;

    char char_input;
    std::cout << "Enter a single character: ";
    std::cin >> char_input;

    std::cout << "\n";

    // ── Implicit conversions (automatic) ────────────────────────────────────────
    std::cout << "Implicit conversions:\n";
    std::cout << "char '" << char_input << "' -> int: " 
              << static_cast<int>(char_input) << " (ASCII value)\n";

    std::cout << "int " << integer_input << " -> double: " 
              << static_cast<double>(integer_input) << "\n";

    std::cout << "double " << decimal_value << " -> int (truncates): " 
              << static_cast<int>(decimal_value) << "\n\n";

    // ── Explicit static_cast (safe & recommended) ───────────────────────────────
    std::cout << "Explicit static_cast examples:\n";

    unsigned int unsigned_from_signed = static_cast<unsigned int>(integer_input);
    std::cout << "int -> unsigned int: " << unsigned_from_signed << "\n";

    int signed_from_double = static_cast<int>(decimal_value);
    std::cout << "double -> int: " << signed_from_double << "\n";

    // Narrowing attempt (may warn depending on flags)
    // int narrow = decimal_value;  // narrowing conversion warning
    // Better: explicit cast shows intent
    int explicit_narrow = static_cast<int>(decimal_value);
    std::cout << "double -> int (explicit): " << explicit_narrow << "\n\n";

    // ── Signed <-> Unsigned pitfalls ──────────────────────────────────────────────
    std::cout << "Signed / Unsigned conversion example:\n";
    int negative = -42;
    unsigned int as_unsigned = static_cast<unsigned int>(negative);
    std::cout << "negative int (-42) -> unsigned int: " << as_unsigned 
              << " (wraps to large value)\n";

    // Safe way: check before conversion
    if (negative >= 0) {
        unsigned int safe_unsigned = static_cast<unsigned int>(negative);
        std::cout << "Safe unsigned conversion: " << safe_unsigned << "\n";
    } else {
        std::cout << "Cannot safely convert negative to unsigned.\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Prefer static_cast over C-style (int)x casts\n"
              << "  * Be careful with narrowing and signed <-> unsigned\n"
              << "  * Always validate when converting between signed/unsigned\n";

    return 0;
}