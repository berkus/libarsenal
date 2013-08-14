cmake_minimum_required(VERSION 2.8)

include(cmake/setup.cmake)

# Additional unofficial boost libraries
include_directories(../../mettanode/3rdparty/endian)

# Support library.
include_directories(include)
add_subdirectory(lib)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
