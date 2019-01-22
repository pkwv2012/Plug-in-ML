
add_library(master master.cc task_config.cc)

target_link_libraries(master zmq_communicator gflags message)

add_executable(master_main master_main.cc)

target_link_libraries(master_main gflags message zmq_communicator master)

add_executable(master_test master_test.cc)
target_link_libraries(master_test gtest_main message zmq_communicator master gflags pthread gtest)
