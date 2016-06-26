cmake_minimum_required(VERSION 2.8.9)
project(language)
set(CMAKE_CXX_FLAGS "-std=c++14 -O3")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(GFlags REQUIRED)
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(LevelDB REQUIRED)

add_subdirectory(proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(source_util util/source_util.h util/source_util.cc)
target_link_libraries(source_util language_proto ${PROTOBUF_LIBRARY})

add_executable(test_main.exe interpreter/test_main.cc interpreter/memory.cc interpreter/literal_ops.cc interpreter/language_evaluation.cc)
target_link_libraries(test_main.exe language_proto source_util gflags)

add_executable(storage_server.exe storage/leveldb_store.cc storage/storage_server.cc storage/storage_server_main.cc)
target_link_libraries(storage_server.exe source_util language_proto gflags ${LevelDB_LIBRARY} ${GRPC_LIBRARIES})

add_executable(storage_tool.exe storage/storage_tool_main.cc)
target_link_libraries(storage_tool.exe language_proto gflags ${GRPC_LIBRARIES})
