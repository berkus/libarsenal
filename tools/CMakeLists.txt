add_executable(log_dump log_dump.cpp)
target_link_libraries(log_dump arsenal ${Boost_LIBRARIES})
install(TARGETS log_dump
    RUNTIME DESTINATION tools)

add_executable(sidelog sidelog.cpp)
target_link_libraries(sidelog arsenal ${Boost_LIBRARIES})
install(TARGETS sidelog
    RUNTIME DESTINATION tools)
