# unitconv – a small library packaged the way a real third-party dependency is.
# It builds on its own (cmake -S . -B build && cmake --build build && cmake --install build --prefix out)
# and inside this course, where day 79 adds it with add_subdirectory().
cmake_minimum_required(VERSION 3.20)
project(unitconv VERSION 2.1.0 DESCRIPTION "Length, mass and temperature conversions" LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(unitconv STATIC src/convert.cpp)
add_library(unitconv::unitconv ALIAS unitconv)  # consumers use the same name as after find_package()
target_compile_features(unitconv PUBLIC cxx_std_20)  # usage requirement: consumers need C++20 too
set_target_properties(unitconv PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

# The version header is generated from the single source of truth: project(VERSION ...).
configure_file(cmake/version.hpp.in "${PROJECT_BINARY_DIR}/generated/unitconv/version.hpp" @ONLY)

# Build tree and install tree have different include paths; generator expressions pick the right one.
target_include_directories(unitconv
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE src)

if(TARGET cppm_settings)  # inside the course: use the course's strict warnings, privately
    # A static library's PRIVATE links still appear in its export (consumers must link them), so a
    # build-only helper target is wrapped in BUILD_INTERFACE to keep it out of the installed package.
    target_link_libraries(unitconv PRIVATE "$<BUILD_INTERFACE:cppm_settings>")
endif()

# ---- install & export: lets other projects write find_package(unitconv 2.0 REQUIRED) ----
install(TARGETS unitconv EXPORT unitconvTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/generated/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT unitconvTargets NAMESPACE unitconv:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unitconv)
configure_package_config_file(cmake/unitconvConfig.cmake.in "${PROJECT_BINARY_DIR}/unitconvConfig.cmake"
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unitconv)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/unitconvConfigVersion.cmake" COMPATIBILITY SameMajorVersion)
install(FILES "${PROJECT_BINARY_DIR}/unitconvConfig.cmake" "${PROJECT_BINARY_DIR}/unitconvConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unitconv)

# ---- CPack: `cpack -G TGZ` in the build directory produces unitconv-2.1.0-<system>.tar.gz ----
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)  # only when built on its own, not inside the course
    set(CPACK_PACKAGE_VENDOR "C++ Systems Architecture course")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
    set(CPACK_GENERATOR TGZ ZIP)
    include(CPack)
endif()
