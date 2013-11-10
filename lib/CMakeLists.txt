set(support_SOURCES
    base32.cpp
    byte_array.cpp
    logging.cpp
    flurry.cpp
    settings_provider.cpp)

if (APPLE)
    list(APPEND support_SOURCES
        settings_provider_osx.cpp)
endif()

if (UNIX AND NOT APPLE)
    list(APPEND support_SOURCES
        settings_provider_linux.cpp)
endif()

add_library(support STATIC ${support_SOURCES})
