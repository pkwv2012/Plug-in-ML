project("rpscc")

cmake_minimum_required(VERSION 3.5)

#------------------------------------------------------------------------------------------------
# Add protobuf compilation support
#------------------------------------------------------------------------------------------------
include("FindProtobuf")
find_package(Protobuf REQUIRED)
message("protobuf_lib=${PROTOBUF_LIBRARY}\n")
message("protobuf_include_dir=${PROTOBUF_INCLUDE_DIR}\n")
message("protobuf_version=${PROTOBUF_VERSION}\n")
include_directories(${PROTOBUF_INCLUDE_DIR})

add_definitions("-std=c++14")

# How to turn on the multi-thread flag of Gflags?
add_subdirectory(third_party/gflags)

add_subdirectory(third_party/gtest)

include_directories(
        "${PROJECT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/third_party"
        "${PROJECT_SOURCE_DIR}/third_party/gtest/googletest/include"
)

# Add directory of libzmq.dylib.
link_directories("/usr/local/lib")
link_directories("/usr/local/opt/python/Frameworks/Python.framework/Versions/3.7/lib/python3.7/config-3.7m-darwin/")

message(${PROJECT_BINARY_DIR})
message(${PROJECT_SOURCE_DIR})

# use zookeeper or not
# set(USE_ZOO "yes")

# generate shared memory test
# set(SHARED_MEMORY_TEST "yes")

add_subdirectory(src/message)
add_subdirectory(src/communication)
add_subdirectory(src/master)
add_subdirectory(src/util)
add_subdirectory(src/server)
add_subdirectory(src/channel)
add_subdirectory(src/agent)
