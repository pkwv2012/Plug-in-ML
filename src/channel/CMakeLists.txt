
add_executable(channel_test channel_test.cc fifo.cc shared_memory.cc)
add_executable(channel_gtest channel_gtest.cc fifo.cc shared_memory.cc)
target_link_libraries(channel_gtest gtest_main gtest gflags)
target_link_libraries(channel_test gflags)

# There is no librt.dylib on 'APPLE' system.
if (UNIX AND NOT APPLE)
  target_link_libraries(channel_gtest rt)
  target_link_libraries(channel_test rt)
endif()

add_custom_command(TARGET channel_gtest
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*py ${PROJECT_BINARY_DIR}/src/channel/
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*.csv ${PROJECT_BINARY_DIR}/src/channel)
add_custom_command(TARGET channel_test
  COMMAND cp ${CMAKE_SOURCE_DIR}/src/channel/*py ${PROJECT_BINARY_DIR}/src/channel/)

add_executable(shared_memory_test shared_memory_test.cc shared_memory.cc)
if (UNIX AND NOT APPLE)
  add_definitions("-I/usr/include/python3.5m -I/usr/include/python3.5m  -Wno-unused-result -Wsign-compare -g -fstack-protector-strong -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes")
  add_definitions("-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions")
  link_directories("/usr/lib/python3.5/config-3.5m-x86_64-linux-gnu")
  link_directories("/usr/lib")
  target_link_libraries(shared_memory_test gtest_main gtest rt python3.5m pthread dl util m)
else(UNIX AND NOT APPLE)
  add_definitions("-I/usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/include/python3.7m -I/usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/include/python3.7m -Wno-unused-result -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG -g -fwrapv -O3 -Wall -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/usr/include -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/Tk.framework/Versions/8.5/Headers")
  link_directories("/usr/local/opt/python/Frameworks/Python.framework/Versions/3.7/lib/python3.7/config-3.7m-darwin/")
  add_definitions("-framework CoreFoundation")
  target_link_libraries(shared_memory_test gtest_main gtest python3.7m dl)
endif(UNIX AND NOT APPLE)

