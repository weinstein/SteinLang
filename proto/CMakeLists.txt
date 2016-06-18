include_directories(${PROTOBUF_INCLUDE_DIRS})

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${ProtoFiles})
PROTOBUF_GENERATE_GRPC_CPP(GRPC_SRCS GRPC_HDRS ${ProtoFiles})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(language_proto ${PROTO_HDRS} ${PROTO_SRCS} ${GRPC_HDRS} ${GRPC_SRCS})
target_link_libraries(language_proto ${CMAKE_THREAD_LIBS_INIT} ${PROTOBUF_LIBRARIES} ${GRPC_LIBRARIES})
