// unitconv public API.
#pragma once

#include <string>

namespace unitconv {

/// Convert @p value between units of the same dimension: m, km, mi, ft (length); g, kg, lb (mass);
/// C, F, K (temperature). Throws std::invalid_argument for unknown or mismatched units.
double convert(double value, const std::string& from, const std::string& to);

/// The library's version as compiled, e.g. "2.1.0" – may differ from the header a consumer sees.
const char* library_version();

}  // namespace unitconv
