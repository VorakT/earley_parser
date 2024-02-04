cmake_minimum_required(VERSION 3.25)
project(earley_parser)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)

enable_testing()

set(CMAKE_CXX_FLAGS "--coverage")

include_directories(../earley_parser)

add_subdirectory(../earley_parser build)  

add_executable(Test EarleyParserTestCase.cpp)

include(GoogleTest)
find_package(GTest REQUIRED)

target_link_libraries(Test EarleyParser)
target_link_libraries(Test GTest::GTest GTest::Main)

gtest_discover_tests(Test)

