# Build protobuf
message("Building protobuf")
set(PROTOBUF_INPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/src/message")
set(PROTOBUF_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/src/message")

set(CURRENT_DIR "${PROJECT_SOURCE_DIR}/src/message")
message("pwd = ${CURRENT_DIR}")

add_custom_command(
        OUTPUT message.pb.h message.pb.cc
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out ${CURRENT_DIR} message.proto
        WORKING_DIRECTORY ${CURRENT_DIR}
        COMMENT "Running C++ protocol buffer compiler"
)

add_custom_target(
        generate_message ALL
        DEPENDS message.pb.h message.pb.cc
        COMMENT "Building custom target"
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROJECT_SOURCE_DIR}/src/message/message.proto)
message("hdr=${PROTO_HDRS} src=${PROTO_SRCS} ...")

#add_library(message message.pb.h message.pb.cc)
