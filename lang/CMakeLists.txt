add_library(
  tokenizer
  tokenizer.h
)
set_target_properties(tokenizer PROPERTIES LINKER_LANGUAGE CXX)

add_library(
  recursive_descent_parser
  recursive_descent_parser.h
  parse_tree_util.h
)
set_target_properties(recursive_descent_parser PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(
  recursive_descent_parser
  tokenizer
  util
)

add_library(
  cfg_parser
  cfg_parser.h cfg_parser.cc
)
target_link_libraries(
  cfg_parser
  recursive_descent_parser
  tokenizer
  util
)

add_executable(
  cfg_parser_main.exe
  cfg_parser_main.cc
)
target_link_libraries(
  cfg_parser_main.exe
  cfg_parser
  gflags
)

add_subdirectory(interpreter)
