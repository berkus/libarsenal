#
# This is generic boilerplate
#
if (NOT __cmake_setup_INCLUDED)
set(__cmake_setup_INCLUDED 1)

# Build in 64 bits mode by default... @fixme
set(CMAKE_CXX_FLAGS "-m64 ${CMAKE_CXX_FLAGS}")

# Enable C++14 and link libc++
set(CMAKE_CXX_FLAGS "-ferror-limit=3 ${CMAKE_CXX_FLAGS} -std=c++1y -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")

# Enable full error and warning reporting
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter")

# Nasty boost.unit_test_framework
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unneeded-internal-declaration")

# Test effect of global constructors in our code, global constructors are usually bad idea.
# Cannot be enabled because boost uses them in boost.error_code
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wglobal-constructors")

# Clang invocation debug
#set(CMAKE_CXX_FLAGS "-v ${CMAKE_CXX_FLAGS}")

option(CLANG "Build using clang." ON)
option(CLANG_GIT "[developer] Build using clang-git." OFF)
option(CLANG_ASAN "Use clang's address-sanitizer." ON)
option(ASAN_PARANOIA "Use clang's address-sanitizer FULL ON." OFF)
option(CLANG_TSAN "Use clang's thread-sanitizer." OFF)
option(CLANG_MSAN "Use clang's memory-sanitizer." OFF)
option(CLANG_USAN "Use clang's UB-sanitizer." OFF)
option(CLANG_LTO "[time-consuming] Enable link-time optimization with LLVM." OFF)
option(CLANG_CHECK_DOCS "[developer] Check documentation consistency using -Wdocumentation" ON)
option(CLANG_ANALYZE "[developer] Run clang in static analyzer mode." OFF)

option(TRAVIS_CI "Build on Travis-CI nodes (disables some warnings)" OFF)
option(TEST_COVERAGE "Enable coverage information generation by the compiler" OFF)

if (CLANG)
    set(_extraFlags)
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

# On mac, use openssl from brew, not the default system one, because it is too old.
# Run 'brew install openssl' to install it.
if (APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    # Set pkg-config path in case pkg-config is installed on the machine.
    set(ENV{PKG_CONFIG_PATH} /usr/local/opt/openssl/lib/pkgconfig)
    # A bug in cmake prevents use of OPENSSL_ROOT_DIR for finding a custom openssl,
    # so we use an internal variable instead. This needs to be fixed in cmake.
    set(_OPENSSL_ROOT_HINTS_AND_PATHS PATHS /usr/local/opt/openssl)
endif (APPLE)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

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
if (UNIX AND NOT APPLE)
    # Filesystem library used on Linux for creating config directories
    list(APPEND BOOST_COMPONENTS filesystem)
endif (UNIX AND NOT APPLE)

#set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON) # Easier to deploy elsewhere
set(BOOST_ROOT /usr/local/opt/boost)
set(BOOST_LIBRARYDIR /usr/local/opt/boost/lib64)
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})

include_directories(${Boost_INCLUDE_DIR})

# Create and link a test application.
function(create_test NAME)
    cmake_parse_arguments(CT "NO_CTEST" "" "LIBS" ${ARGN})
    add_executable(test_${NAME} test_${NAME}.cpp)
    target_link_libraries(test_${NAME} ${CT_LIBS} ${Boost_LIBRARIES})
    if (UNIX AND NOT APPLE)
        target_link_libraries(test_${NAME} c++)
    endif()
    install(TARGETS test_${NAME}
        RUNTIME DESTINATION tests/unittests)
    if (NOT CT_NO_CTEST)
        add_test(${NAME} test_${NAME})
    endif (NOT CT_NO_CTEST)
endfunction(create_test)

endif (NOT __cmake_setup_INCLUDED)
