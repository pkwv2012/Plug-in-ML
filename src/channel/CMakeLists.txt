add_library(channel fifo.cc sharedmemory.cc)
add_executable(test channel_test.cc)
target_link_libraries(test channel)