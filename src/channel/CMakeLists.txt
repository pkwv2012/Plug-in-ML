#add_library(channel fifo.cc sharedmemory.cc)
add_executable(channel_test channel_test.cc fifo.cc shared_memory.cc)
#target_link_libraries(channel_test cannel)