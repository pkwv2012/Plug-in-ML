
add_library(master master.cc)

add_executable(master_main master_main.cc)

target_link_libraries(master_main master gflags message)
