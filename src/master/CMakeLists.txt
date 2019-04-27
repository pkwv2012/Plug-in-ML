
# use zookeeper or not
# add_definitions("-DUSE_ZOOKEEPER")

add_library(master master.cc task_config.cc)

target_link_libraries(master zmq_communicator gflags message logging)

if (USE_ZOO)
  target_link_libraries(master zookeeper_mt)
endif(USE_ZOO)

add_executable(master_main master_main.cc)

target_link_libraries(master_main gflags message zmq_communicator master logging)

add_executable(master_test master_test.cc)
target_link_libraries(master_test gtest_main message zmq_communicator master gflags pthread gtest logging)

add_executable(test_process test_process.cc)
target_link_libraries(test_process message zmq_communicator master gflags pthread gtest logging)

if (USE_ZOO)
  add_executable(zk_test zookeeper_test.cc)
  target_link_libraries(zk_test zookeeper_mt)
endif(USE_ZOO)