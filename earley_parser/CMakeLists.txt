cmake_minimum_required(VERSION 3.25)
project(earley_parser)

set(CMAKE_CXX_STANDARD 20)

add_library(EarleyParser EarleyParser.h
	EarleyParser.cpp)

