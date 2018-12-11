project("rpscc")

cmake_minimum_required(VERSION 3.9)

#------------------------------------------------------------------------------------------------
# Add protobuf compilation support
#------------------------------------------------------------------------------------------------
include("FindProtobuf")
find_package(Protobuf REQUIRED)

add_definitions("-std=c++11")

include_directories(
        "${PROJECT_SOURCE_DIR}"
)

add_subdirectory(src/master)
add_subdirectory(src/message)