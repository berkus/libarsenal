add_executable(log_dump log_dump.cpp hexdump.cpp)
target_link_libraries(log_dump support ${Boost_LIBRARIES})
install(TARGETS log_dump
    RUNTIME DESTINATION tools)
