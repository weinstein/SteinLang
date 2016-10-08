cmake_minimum_required(VERSION 2.8.9)
project(language)
set(CMAKE_CXX_FLAGS "-std=c++14 -O3")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(GFlags REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(util)
add_subdirectory(lang)
