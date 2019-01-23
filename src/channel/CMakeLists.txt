
add_executable(channel_test channel_test.cc fifo.cc shared_memory.cc)
add_executable(channel_gtest channel_gtest.cc fifo.cc shared_memory.cc)
target_link_libraries(channel_gtest gtest_main gtest rt)
target_link_libraries(channel_test rt)
add_custom_command(TARGET channel_gtest
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*py ${PROJECT_BINARY_DIR}/src/channel/
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*.csv ${PROJECT_BINARY_DIR}/src/channel)
add_custom_command(TARGET channel_test
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*py ${PROJECT_BINARY_DIR}/src/channel/)
