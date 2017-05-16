#
# This is generic boilerplate
#
if (NOT __cmake_setup_INCLUDED)
set(__cmake_setup_INCLUDED 1)

option(CLANG "Build using clang." ON)
option(CLANG_GIT "[developer] Build using clang-git." OFF)
option(CLANG_ASAN "Use clang's address-sanitizer." OFF)
option(ASAN_PARANOIA "Use clang's address-sanitizer FULL ON." OFF)
option(CLANG_TSAN "Use clang's thread-sanitizer." OFF)
option(CLANG_MSAN "Use clang's memory-sanitizer." OFF)
option(CLANG_USAN "Use clang's UB-sanitizer." OFF)
option(CLANG_LTO "[time-consuming] Enable link-time optimization with LLVM." OFF)
option(CLANG_CHECK_DOCS "[developer] Check documentation consistency using -Wdocumentation" OFF)
option(CLANG_ANALYZE "[developer] Run clang in static analyzer mode." OFF)
option(CLANG_DEBUG "[developer] Debug clang invocations." OFF)

option(TRAVIS_CI "Build on Travis-CI nodes (disables some warnings)" OFF)
option(TEST_COVERAGE "Enable coverage information generation by the compiler" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message("Supported features = ${CMAKE_CXX_COMPILE_FEATURES}")

set(CMAKE_CXX_FLAGS "-ferror-limit=3 ${CMAKE_CXX_FLAGS}")

# and link libc++
if (APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()

# Enable full error and warning reporting
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

# Test effect of global constructors in our code, global constructors are usually bad idea.
# Cannot be enabled because boost uses them in boost.error_code
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wglobal-constructors")

if (CLANG)
    # Nasty boost.unit_test_framework
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unneeded-internal-declaration")
    # Nasty boost.asio unused typedefs
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef")

    set(_extraFlags)
    if (CLANG_DEBUG)
        set(_extraFlags "-v")
    endif()
    if (CLANG_ANALYZE)
        set(_extraFlags "${_extraFlags} --analyze -Xanalyzer -analyzer-output=text")
    endif ()
    if (CLANG_ASAN)
        if (ASAN_PARANOIA)
            set(_extraFlags "${_extraFlags} -fsanitize=address-full") # EXPENSIVE!
        else()
            set(_extraFlags "${_extraFlags} -fsanitize=address")
        endif()
        # Add ASAN_OPTIONS="detect_leaks=1" to cmdline to track leaks! - 2013.10.16
    endif ()
    if (CLANG_TSAN)
        set(_extraFlags "${_extraFlags} -fsanitize=thread")
    endif ()
    if (CLANG_MSAN)
        set(_extraFlags "${_extraFlags} -fsanitize=memory -fsanitize-memory-track-origins")
    endif ()
    if (CLANG_USAN)
        set(_extraFlags "${_extraFlags} -fsanitize=undefined")
    endif ()
    if (CLANG_CHECK_DOCS)
        set(_extraFlags "${_extraFlags} -Wdocumentation")
    endif ()
    # Some unused arguments here and there
    set(_extraFlags "${_extraFlags} -Wno-unused-parameter -Wno-unused-private-field")

    if (CLANG_LTO)
        set(_extraFlags "${_extraFlags} -emit-llvm")
        set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -O4")
    endif ()
    if (TRAVIS_CI)
        set(_extraFlags "${_extraFlags} -Wno-uninitialized") # Qt code in travis-ci nodes is lame.
        set(_extraFlags "${_extraFlags} -Wno-documentation") # boost doxygen documentation is lame.
        # Qt4 DOES ACTUALLY USE register keyword...
        set(_extraFlags "${_extraFlags} -Wno-deprecated-register")
    endif ()

    if (CLANG_GIT)
        if (NOT LIBCXX_BASE)
            message(FATAL_ERROR "Need to specify non-default libc++ location.")
        endif (NOT LIBCXX_BASE)
        message(STATUS "ENV{DYLD_LIBRARY_PATH} $ENV{DYLD_LIBRARY_PATH}")
        #if (NOT $ENV{DYLD_LIBRARY_PATH})
        #   message(FATAL_ERROR "Did you forget to set custom clang DYLD_LIBRARY_PATH location?")
        #endif (NOT $ENV{DYLD_LIBRARY_PATH})

        # Include local libcxx first.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -iwithprefixbefore${LIBCXX_BASE}/include/c++/v1")
    endif (CLANG_GIT)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_extraFlags}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_extraFlags}")
endif (CLANG)

if (TEST_COVERAGE)
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
    endif (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
endif (TEST_COVERAGE)

include(FindSodium)
include_directories(${SODIUM_INCLUDE_DIRS})

set(BOOST_COMPONENTS)

if (BUILD_TESTING)
    list(APPEND BOOST_COMPONENTS unit_test_framework)
    enable_testing()
    include(Dart)
endif (BUILD_TESTING)

# boost/asio depends on libboost_system
list(APPEND BOOST_COMPONENTS system)
# For logging we need boost/posix_time
list(APPEND BOOST_COMPONENTS date_time)
# Program_options used to parse cmdline args in some tests
list(APPEND BOOST_COMPONENTS program_options)
# Thread library used in some nat libs and tests
list(APPEND BOOST_COMPONENTS thread)
# Filesystem library used on Linux for creating config directories
list(APPEND BOOST_COMPONENTS filesystem)

list(APPEND BOOST_COMPONENTS iostreams)
list(APPEND BOOST_COMPONENTS log)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
include_directories(${Boost_INCLUDE_DIR})

# For zlib-compressed iostreams
list(APPEND Boost_LIBRARIES z)

# Create and link a test application.
function(create_test NAME)
    cmake_parse_arguments(CT "NO_CTEST" "" "LIBS" ${ARGN})
    add_executable(test_${NAME} test_${NAME}.cpp)
    target_link_libraries(test_${NAME} ${CT_LIBS} ${Boost_LIBRARIES})
    if (UNIX AND NOT APPLE)
        target_link_libraries(test_${NAME} pthread)
    endif()
    install(TARGETS test_${NAME}
        RUNTIME DESTINATION tests/unittests)
    if (NOT CT_NO_CTEST)
        add_test(${NAME} test_${NAME})
    endif (NOT CT_NO_CTEST)
endfunction(create_test)

endif (NOT __cmake_setup_INCLUDED)
