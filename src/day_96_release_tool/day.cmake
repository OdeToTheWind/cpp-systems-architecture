# Day 96: the version of "logslice" is defined once, here. CMake writes it into a generated header,
# and the tests check that the changelog and the committed usage docs agree with it.
set(LOGSLICE_VERSION "1.3.0")
configure_file("${DAY_DIR}/release/version.hpp.in" "${CMAKE_BINARY_DIR}/generated/day96/logslice_version.hpp" @ONLY)

add_library(logslice_meta INTERFACE)
target_include_directories(logslice_meta INTERFACE "${CMAKE_BINARY_DIR}/generated/day96")
# Lets the tests find CHANGELOG.md and USAGE.md in the source tree, wherever the build directory is.
target_compile_definitions(logslice_meta INTERFACE LOGSLICE_SOURCE_DIR="${DAY_DIR}")
set(DAY_LINK_LIBRARIES logslice_meta)
