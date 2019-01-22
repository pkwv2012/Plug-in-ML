
add_library(agent agent.cc partition.cc ../channel/fifo.cc ../channel/shared_memory.cc)
target_link_libraries(agent gflags message zmq_communicator)

add_executable(agent_test agent_test.cc)
target_link_libraries(agent_test agent rt)

add_executable(agent_gtest agent_gtest.cc)
target_link_libraries(agent_gtest gtest_main agent rt)
