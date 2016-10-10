file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
add_library(
  steinlang_syntax_proto
  ${PROTO_HDRS} ${PROTO_SRCS}
)
target_link_libraries(
  steinlang_syntax_proto
  ${PROTOBUF_LIBRARIES}
)

add_library(
  language_evaluation
  memory.h memory.cc
  literal_ops.h literal_ops.cc
  language_evaluation.h language_evaluation.cc
)
target_link_libraries(
  language_evaluation
  steinlang_syntax_proto
  util
  ${PROTOBUF_LIBRARIES}
)

add_executable(
  interpreter_main.exe
  interpreter_main.cc
  source_util.h source_util.cc
)
target_link_libraries(
  interpreter_main.exe
  language_evaluation
  util
  gflags
)

add_library(
  steinlang_parser
  steinlang_parser.h
  steinlang_parser.cc
)
target_link_libraries(
  steinlang_parser
  tokenizer
  recursive_descent_parser
)
