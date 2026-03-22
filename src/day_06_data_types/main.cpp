#include <iostream>
#include <iomanip>          // setw
#include <cstdint>          // fixed-width types
#include <limits>           // numeric_limits
#include <cstddef>          // size_t, ptrdiff_t

// Type alias example (modern C++ style)
using byte = std::uint8_t;
using index_t = std::size_t;

int main() {
    std::cout << "=== Day 06: Data Types & Their Properties ===\n\n";

    std::cout << std::left << std::setw(20) << "Type"
              << std::setw(12) << "Size (bytes)"
              << std::setw(30) << "Min Value"
              << "Max Value\n";
    std::cout << std::string(72, '-') << "\n";

    // Fundamental types
    std::cout << std::setw(20) << "char" 
              << std::setw(12) << sizeof(char)
              << std::numeric_limits<char>::min() << " → "
              << std::numeric_limits<char>::max() << "\n";

    std::cout << std::setw(20) << "unsigned char"
              << std::setw(12) << sizeof(unsigned char)
              << 0 << " → "
              << std::numeric_limits<unsigned char>::max() << "\n";

    std::cout << std::setw(20) << "short"
              << std::setw(12) << sizeof(short)
              << std::numeric_limits<short>::min() << " → "
              << std::numeric_limits<short>::max() << "\n";

    std::cout << std::setw(20) << "int"
              << std::setw(12) << sizeof(int)
              << std::numeric_limits<int>::min() << " → "
              << std::numeric_limits<int>::max() << "\n";

    std::cout << std::setw(20) << "long long"
              << std::setw(12) << sizeof(long long)
              << std::numeric_limits<long long>::min() << " → "
              << std::numeric_limits<long long>::max() << "\n";

    std::cout << std::setw(20) << "float"
              << std::setw(12) << sizeof(float)
              << std::numeric_limits<float>::lowest() << " → "
              << std::numeric_limits<float>::max() << "\n";

    std::cout << std::setw(20) << "double"
              << std::setw(12) << sizeof(double)
              << std::numeric_limits<double>::lowest() << " → "
              << std::numeric_limits<double>::max() << "\n\n";

    // Fixed-width types (exact size guaranteed)
    std::cout << "Fixed-width types (from <cstdint>):\n";
    std::cout << "int32_t  size: " << sizeof(int32_t) << " bytes\n";
    std::cout << "uint64_t size: " << sizeof(uint64_t) << " bytes\n";
    std::cout << "size_t   size: " << sizeof(size_t)   << " bytes (platform-dependent)\n\n";

    // ── Signed vs Unsigned overflow behavior ────────────────────────────────────
    int32_t signed_val = std::numeric_limits<int32_t>::max();
    std::cout << "Signed max: " << signed_val << "\n";
    // signed_val++;  // ← undefined behavior! Do NOT do this

    uint32_t unsigned_val = std::numeric_limits<uint32_t>::max();
    std::cout << "Unsigned max: " << unsigned_val << "\n";
    unsigned_val++;  // wraps around — defined behavior
    std::cout << "After ++ (wrap): " << unsigned_val << " (0)\n\n";

    // ── Type alias usage example ────────────────────────────────────────────────
    byte small_number = 255;
    index_t array_length = 1000000;

    std::cout << "byte alias (uint8_t): " << static_cast<int>(small_number) << "\n";
    std::cout << "index_t alias (size_t): " << array_length << "\n";

    std::cout << "\nKey takeaway: Use fixed-width types when exact size matters.\n";
    std::cout << "Prefer double over float for most calculations.\n";

    return 0;
}