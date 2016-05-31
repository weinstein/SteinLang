cmake_minimum_required(VERSION 2.8.9)
project(language)
set(CMAKE_CXX_FLAGS "-std=c++11 -O3")

find_package(GFlags REQUIRED)

add_subdirectory(proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(bin/test_main interpreter/test_main.cc interpreter/language_evaluation.cc interpreter/literal_ops.cc interpreter/memory.cc)
target_link_libraries(bin/test_main gflags language_proto ${PROTOBUF_LIBRARY})
