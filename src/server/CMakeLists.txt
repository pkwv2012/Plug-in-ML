
add_library(server server.cc pull_info.cc key_value_list.cc)
target_link_libraries(server gflags message zmq_communicator logging)

add_executable(server_main server_main.cc)
target_link_libraries(server_main server logging)

add_executable(server_test server_test.cc)
target_link_libraries(server_test server message)

add_executable(server_gtest server_gtest.cc)
target_link_libraries(server_gtest gtest_main server message)

if (UNIX AND NOT APPLE)
  target_link_libraries(server_main rt)
  target_link_libraries(server_test rt)
  target_link_libraries(server_gtest rt)
endif()
