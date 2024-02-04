cmake_minimum_required(VERSION 3.25)
project(earley_parser)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(earley_parser)

add_executable(Main main.cpp)

target_link_libraries(Main EarleyParser)
