#add_library(logging logging.h)

add_library(logging logging.cc)
target_link_libraries(logging)

add_executable(logging_test logging_test.cc)
target_link_libraries(logging_test gtest_main gtest logging)
