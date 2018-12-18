project("rpscc")

cmake_minimum_required(VERSION 3.9)

#------------------------------------------------------------------------------------------------
# Add protobuf compilation support
#------------------------------------------------------------------------------------------------
include("FindProtobuf")
find_package(Protobuf REQUIRED)

add_definitions("-std=c++14")

# How to turn on the multi-thread flag of Gflags?
add_subdirectory(thrid_party/gflags)

include_directories(
        "${PROJECT_SOURCE_DIR}"
        "thrid_party"
)

add_subdirectory(src/communication)
add_subdirectory(src/master)
add_subdirectory(src/message)