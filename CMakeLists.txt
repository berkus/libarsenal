cmake_minimum_required(VERSION 2.8)

if (NOT BUILD_IN_METTANODE)
    include(cmake/setup.cmake)
endif()

# Additional unofficial boost libraries
include_directories(../../mettanode/3rdparty/endian)

# Support library.
include_directories(include)
add_subdirectory(lib)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
