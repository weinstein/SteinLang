add_library(
  diff_proto
  diff_proto.h diff_proto.cc
)
target_link_libraries(
  diff_proto
  util
  ${PROTOBUF_LIBRARIES}
)
