
add_library(server server.cc pull_info.cc key_value_list.cc)
target_link_libraries(server gflags message zmq_communicator)

add_executable(server_main server_main.cc)
target_link_libraries(server_main server rt)

add_executable(server_test server_test.cc)
target_link_libraries(server_test server message rt)
