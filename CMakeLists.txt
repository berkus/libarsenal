cmake_minimum_required(VERSION 3.8)

if (NOT BUILD_IN_UVVY)
    include(cmake/setup.cmake)
endif()

# Support library.
include_directories(include)
add_subdirectory(lib)
add_subdirectory(tools)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
