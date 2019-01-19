
add_library(master master.cc task_config.cc master.h task_config.h)

add_executable(master_main master_main.cc)

target_link_libraries(master_main master gflags message zmq_communicator)
