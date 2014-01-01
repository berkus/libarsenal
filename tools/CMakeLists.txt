add_executable(log_dump log_dump.cpp hexdump.cpp)
target_link_libraries(log_dump support ${Boost_LIBRARIES})
install(TARGETS log_dump
    RUNTIME DESTINATION tools)

add_executable(sidelog sidelog.cpp hexdump.cpp)
target_link_libraries(sidelog support ${Boost_LIBRARIES})
install(TARGETS sidelog
    RUNTIME DESTINATION tools)
