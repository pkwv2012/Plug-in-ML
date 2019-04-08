
add_executable(channel_test channel_test.cc fifo.cc shared_memory.cc)
add_executable(channel_gtest channel_gtest.cc fifo.cc shared_memory.cc)
target_link_libraries(channel_gtest gtest_main gtest rt gflags)
target_link_libraries(channel_test rt gflags)
add_custom_command(TARGET channel_gtest
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*py ${PROJECT_BINARY_DIR}/src/channel/
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*.csv ${PROJECT_BINARY_DIR}/src/channel)
add_custom_command(TARGET channel_test
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*py ${PROJECT_BINARY_DIR}/src/channel/)

add_executable(shared_memory_test shared_memory_test.cc shared_memory.cc)
add_definitions("-I/usr/include/python3.5m -I/usr/include/python3.5m  -Wno-unused-result -Wsign-compare -g -fstack-protector-strong -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes")
add_definitions("-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions")
link_directories("/usr/lib/python3.5/config-3.5m-x86_64-linux-gnu")
link_directories("/usr/lib")

target_link_libraries(shared_memory_test gtest_main gtest rt python3.5m pthread dl util m)