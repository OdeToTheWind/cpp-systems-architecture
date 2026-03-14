cmake_minimum_required(VERSION 3.20)

project(Cpp100Days VERSION 0.1 LANGUAGES CXX)

# ── Global settings applied to all targets ──────────────────────────────────────
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Common warnings (GCC/Clang + MSVC friendly)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
elseif (MSVC)
    add_compile_options(/W4 /permissive- /wd4996)
endif()

# ── Automatically discover and configure each day_XX_* folder ───────────────────
file(GLOB DAY_DIRS RELATIVE ${CMAKE_SOURCE_DIR} "src/day_*")

foreach(DAY_DIR ${DAY_DIRS})
    # Full path to the day's folder
    set(FULL_DAY_DIR "${CMAKE_SOURCE_DIR}/${DAY_DIR}")

    # Get the day name (day_01_memory, day_02_strings, ...)
    get_filename_component(DAY_NAME ${FULL_DAY_DIR} NAME)

    # Find all .cpp files in this day's folder
    file(GLOB DAY_SOURCES "${FULL_DAY_DIR}/*.cpp")

    # Skip if no sources found
    list(LENGTH DAY_SOURCES SOURCES_COUNT)
    if(SOURCES_COUNT EQUAL 0)
        message(STATUS "Skipping ${DAY_NAME} - no .cpp files found")
        continue()
    endif()

    # Create executable
    add_executable(${DAY_NAME} ${DAY_SOURCES})

    # Includes: global + day-specific
    target_include_directories(${DAY_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${FULL_DAY_DIR}                # for local headers like "string_utils.hpp"
    )

    message(STATUS "Configured executable → ${DAY_NAME} with ${SOURCES_COUNT} source file(s)")
endforeach()

# Optional: summary
message(STATUS "Configured ${CMAKE_ARGC} day executables")