cmake_minimum_required(VERSION 3.19.0)

project(hello)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
message("CMAKE_INSTALL_RPATH = " ${CMAKE_INSTALL_RPATH})

add_library(archive STATIC lib/factorial.cpp)
#target_include_directories(archive PUBLIC lib)

add_executable(hello hello.cpp)
target_link_libraries(hello archive)
target_include_directories(hello PRIVATE lib)
