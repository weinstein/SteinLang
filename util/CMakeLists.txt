add_library(
  util
  file_io.h
  one_of.h
  optional.h
  strings.h
)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
