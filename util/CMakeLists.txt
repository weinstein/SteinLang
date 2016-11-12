add_library(
  util
  file_io.h
  one_of.h
  optional.h
  strings.h
  diff.h
)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)

add_executable(
  diff_tool.exe
  diff_tool.cc
)
target_link_libraries(
  diff_tool.exe
  util
  gflags
)
