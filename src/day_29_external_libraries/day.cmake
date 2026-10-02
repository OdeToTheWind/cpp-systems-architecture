# Day 29 builds its own small library, "textkit", exactly as a third-party project would:
# a STATIC library target with a version, public headers and usage requirements.
add_library(textkit STATIC "${DAY_DIR}/textkit/stats.cpp")
target_include_directories(textkit PUBLIC "${DAY_DIR}")   # consumers write #include "textkit/stats.hpp"
target_link_libraries(textkit PRIVATE cppm_settings)      # our warning flags, not exported to users
set_target_properties(textkit PROPERTIES VERSION 1.4.2 SOVERSION 1)

set(DAY_SKIP_DIRS "${DAY_DIR}/textkit")   # textkit's .cpp files are not part of the lesson library
set(DAY_LINK_LIBRARIES textkit)           # …but the lesson links against it
