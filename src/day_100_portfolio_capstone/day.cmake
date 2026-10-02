# Day 100: the budget tool's version is single-sourced here, like a real release.
set(BUDGET_VERSION "1.0.0")
configure_file("${DAY_DIR}/budget/version.hpp.in" "${CMAKE_BINARY_DIR}/generated/day100/budget_version.hpp" @ONLY)
add_library(budget_meta INTERFACE)
target_include_directories(budget_meta INTERFACE "${CMAKE_BINARY_DIR}/generated/day100")
set(DAY_LINK_LIBRARIES budget_meta)
