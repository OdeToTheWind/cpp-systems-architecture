# Day 79 consumes "unitconv", a library with its own CMake project, install/export rules and CPack
# configuration. add_subdirectory() builds it as part of this tree; another project would instead
# install it and call find_package(unitconv 2.0 REQUIRED).
add_subdirectory("${DAY_DIR}/unitconv" "${CMAKE_BINARY_DIR}/unitconv")

set(DAY_SKIP_DIRS "${DAY_DIR}/unitconv")
set(DAY_LINK_LIBRARIES unitconv::unitconv)
