# FindSodium.cmake
if (NOT SODIUM_FOUND)
    find_path(SODIUM_INCLUDE_DIRS
        NAMES sodium.h
        PATHS
            ${SODIUM_PREFIX}/include
            /usr/include
            /usr/include/sodium
            /usr/local/include
            /opt/local/include
            /usr/local/opt/sodium/include/
        NO_DEFAULT_PATH
    )
    find_library(SODIUM_LIBRARIES
        NAMES libsodium.a
        PATHS
            ${SODIUM_PREFIX}/lib
            /usr/lib
            /usr/lib/sodium
            /usr/local/lib
            /usr/local/lib/sodium
            /opt/local/lib
            /usr/local/opt/sodium/lib/
        NO_DEFAULT_PATH
    )
    if (SODIUM_INCLUDE_DIRS AND SODIUM_LIBRARIES)
        message(STATUS "Found Sodium")
        set(SODIUM_FOUND TRUE)
    endif()
endif()
