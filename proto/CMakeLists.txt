include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER language.proto)
add_library(language_proto ${PROTO_HEADER} ${PROTO_SRC})
