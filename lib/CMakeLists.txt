set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder") # boost, seriously?

set(arsenal_SOURCES
    base32.cpp
    base32x.cpp
    base64.cpp
    byte_array.cpp
    logging.cpp
    flurry.cpp
    settings_provider.cpp)

if (APPLE)
    list(APPEND arsenal_SOURCES
        settings_provider_osx.cpp)
endif()

if (UNIX AND NOT APPLE)
    list(APPEND arsenal_SOURCES
        settings_provider_linux.cpp)
endif()

add_library(arsenal STATIC ${arsenal_SOURCES})
