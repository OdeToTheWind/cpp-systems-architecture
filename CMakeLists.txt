cmake_minimum_required(VERSION 3.20)

project(CppSystemsArchitecture
    VERSION 1.0.0
    DESCRIPTION "100 days of professional C++, from the first variable to production-ready tools"
    LANGUAGES CXX)

# ── Options ────────────────────────────────────────────────────────────────────
option(CPPM_WERROR   "Treat compiler warnings as errors"               OFF)
option(CPPM_SANITIZE "Build with AddressSanitizer + UBSan (GCC/Clang)" OFF)
option(CPPM_COVERAGE "Instrument for gcov/gcovr coverage (GCC/Clang)"  OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

# ── One interface target carries every project-wide compile setting ───────────
add_library(cppm_settings INTERFACE)
target_include_directories(cppm_settings INTERFACE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(cppm_settings INTERFACE Threads::Threads)

if(MSVC)
    # /Zc:preprocessor: the standard-conforming preprocessor (the legacy one breaks raw strings in macro arguments)
    target_compile_options(cppm_settings INTERFACE /W4 /permissive- /utf-8 /EHsc /Zc:__cplusplus /Zc:preprocessor)
    target_compile_definitions(cppm_settings INTERFACE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    if(CPPM_WERROR)
        target_compile_options(cppm_settings INTERFACE /WX)
    endif()
else()
    target_compile_options(cppm_settings INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
        -Wnon-virtual-dtor -Woverloaded-virtual)
    if(CPPM_WERROR)
        target_compile_options(cppm_settings INTERFACE -Werror)
    endif()
    if(CPPM_SANITIZE)
        target_compile_options(cppm_settings INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer
                                                       -fno-sanitize-recover=all)
        target_link_options(cppm_settings INTERFACE -fsanitize=address,undefined)
    endif()
    if(CPPM_COVERAGE)
        # atomic counters: the concurrency days update coverage counters from many threads at once
        target_compile_options(cppm_settings INTERFACE --coverage -O0 -fprofile-update=atomic)
        target_link_options(cppm_settings INTERFACE --coverage)
    endif()
endif()

# ── The dependency-free test framework (include/cppm/testing.hpp) ────────────
add_library(cppm_test_main STATIC include/cppm/testing_main.cpp)
target_link_libraries(cppm_test_main PUBLIC cppm_settings)

enable_testing()

# ── Discover src/day_XX_<topic>/ folders ──────────────────────────────────────
# Every day becomes three targets:
#   day_XX_<topic>_lib   the lesson (INTERFACE when header-only, STATIC when it has .cpp files)
#   day_XX_<topic>       the demo executable built from main.cpp
#   test_day_XX          the unit tests from tests/test_day_XX.cpp, registered with CTest
# A day may define extra targets first in an optional src/day_XX_<topic>/day.cmake file. It can set
#   DAY_SKIP_DIRS       folders whose .cpp files belong to its own targets, not to the lesson
#   DAY_LINK_LIBRARIES  targets the lesson links against
file(GLOB DAY_DIRS LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/day_*")
list(SORT DAY_DIRS)
set(CPPM_DAY_COUNT 0)

foreach(DAY_DIR IN LISTS DAY_DIRS)
    if(NOT IS_DIRECTORY "${DAY_DIR}" OR NOT EXISTS "${DAY_DIR}/main.cpp")
        continue()
    endif()
    get_filename_component(DAY_NAME "${DAY_DIR}" NAME)
    string(REGEX MATCH "^day_([0-9]+)_" _ "${DAY_NAME}")
    set(DAY_NUMBER "${CMAKE_MATCH_1}")
    set(DAY_LIB "${DAY_NAME}_lib")

    set(DAY_SKIP_DIRS "")
    set(DAY_LINK_LIBRARIES "")
    if(EXISTS "${DAY_DIR}/day.cmake")
        include("${DAY_DIR}/day.cmake")
    endif()

    file(GLOB_RECURSE LESSON_SOURCES CONFIGURE_DEPENDS "${DAY_DIR}/*.cpp")
    list(REMOVE_ITEM LESSON_SOURCES "${DAY_DIR}/main.cpp")
    foreach(SKIP_DIR IN LISTS DAY_SKIP_DIRS)
        list(FILTER LESSON_SOURCES EXCLUDE REGEX "^${SKIP_DIR}/")
    endforeach()
    if(LESSON_SOURCES)
        add_library(${DAY_LIB} STATIC ${LESSON_SOURCES})
        target_link_libraries(${DAY_LIB} PUBLIC cppm_settings ${DAY_LINK_LIBRARIES})
        target_include_directories(${DAY_LIB} PUBLIC "${DAY_DIR}")
    else()
        add_library(${DAY_LIB} INTERFACE)
        target_link_libraries(${DAY_LIB} INTERFACE cppm_settings ${DAY_LINK_LIBRARIES})
        target_include_directories(${DAY_LIB} INTERFACE "${DAY_DIR}")
    endif()

    add_executable(${DAY_NAME} "${DAY_DIR}/main.cpp")
    target_link_libraries(${DAY_NAME} PRIVATE ${DAY_LIB})

    set(TEST_SOURCE "${CMAKE_SOURCE_DIR}/tests/test_day_${DAY_NUMBER}.cpp")
    if(EXISTS "${TEST_SOURCE}")
        add_executable(test_day_${DAY_NUMBER} "${TEST_SOURCE}")
        target_link_libraries(test_day_${DAY_NUMBER} PRIVATE ${DAY_LIB} cppm_test_main)
        add_test(NAME test_day_${DAY_NUMBER} COMMAND test_day_${DAY_NUMBER})
        set_tests_properties(test_day_${DAY_NUMBER} PROPERTIES LABELS "day" TIMEOUT 120)
    endif()

    math(EXPR CPPM_DAY_COUNT "${CPPM_DAY_COUNT} + 1")
endforeach()

message(STATUS "Configured ${CPPM_DAY_COUNT} day(s) with demo and test executables")

# ── Repository consistency checks (syllabus ↔ code ↔ tests ↔ docs ↔ quizzes) ──
find_package(Python3 3.10 COMPONENTS Interpreter)
if(Python3_FOUND)
    add_test(NAME tooling
             COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s tests/tooling -q
             WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_tests_properties(tooling PROPERTIES LABELS "tooling" TIMEOUT 300
                         ENVIRONMENT "CPPM_BUILD_DIR=${CMAKE_BINARY_DIR}")
else()
    message(STATUS "Python 3.10+ not found: the tooling consistency test is skipped")
endif()
