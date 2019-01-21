# Build protobuf

set(protobuf_generated_path "${PROJECT_SOURCE_DIR}/src/message")
message("protobuf generated path = ${protobuf_generated_path}")
add_library(message message.pb.cc)
target_link_libraries(message ${PROTOBUF_LIBRARY})

add_custom_command(
        OUTPUT "${protobuf_generated_path}/message.pb.h"
               "${protobuf_generated_path}/message.pb.cc"
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out ${protobuf_generated_path} message.proto
        WORKING_DIRECTORY ${protobuf_generated_path}
        COMMENT "Running C++ protocol buffer compiler"
)

#add_custom_target(
#        generate_message ALL
#        DEPENDS message.pb.h message.pb.cc
#        COMMENT "Building custom target"
#)

add_executable(message_test message_test.cc)
target_link_libraries(message_test message)

