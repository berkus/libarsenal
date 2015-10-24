cmake_minimum_required(VERSION 2.8)

if (NOT BUILD_IN_UVVY)
    include(cmake/setup.cmake)
    include_directories(endian)
    # sodiumpp:
    include_directories(.)
    add_subdirectory(sodiumpp)
endif()

# Support library.
include_directories(include)
add_subdirectory(lib)
add_subdirectory(tools)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
