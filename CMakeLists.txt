project(test)

cmake_minimum_required(VERSION 3.22)
add_executable(main src/main.cpp)
set (CMAKE_CXX_STANDARD 20)
target_include_directories(main PRIVATE inc)
