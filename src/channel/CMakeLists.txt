
add_executable(channel_test channel_test.cc fifo.cc shared_memory.cc)
add_executable(channel_gtest channel_gtest.cc fifo.cc shared_memory.cc)
target_link_libraries(channel_gtest gtest_main gtest rt)
target_link_libraries(channel_test rt)