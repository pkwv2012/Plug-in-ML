# Build protobuf

set(temp_CURRENT_DIR "${PROJECT_SOURCE_DIR}/src/message")
message("pwd = ${temp_CURRENT_DIR}")
add_library(message message.pb.cc)

#add_custom_command(
#        OUTPUT message.pb.h message.pb.cc
#        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out ${temp_CURRENT_DIR} message.proto
#        WORKING_DIRECTORY ${temp_CURRENT_DIR}
#        COMMENT "Running C++ protocol buffer compiler"
#)

#add_custom_target(
#        generate_message ALL
#        DEPENDS message.pb.h message.pb.cc
#        COMMENT "Building custom target"
#)

#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROJECT_SOURCE_DIR}/src/message/message.proto)
#message("hdr=${PROTO_HDRS} src=${PROTO_SRCS} ...")

