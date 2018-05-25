cmake_minimum_required(VERSION 3.9)

project(big_integer)
include(gtest.cmake)

set(CMAKE_CXX_STANDARD 17)
set(COVERAGE OFF CACHE BOOL "Coverage")

include_directories(${big_integer_SOURCE_DIR})

add_executable(big_integer_testing
        ./big_integer_testing.cpp
        ./big_integer.h
        ./big_integer.cpp
        ./gtest/gtest-all.cc
        ./gtest/gtest.h
        ./gtest/gtest_main.cc)


target_compile_options(big_integer_testing PRIVATE -std=c++14 -g -Wall -Wextra -Wpedantic -Wconversion -Wswitch-default -Wswitch-enum -Wunreachable-code -Wwrite-strings -Wcast-align -Wshadow -Wundef)

if (COVERAGE)
    target_compile_options(tests PRIVATE --coverage)
    target_link_libraries(tests PRIVATE --coverage)
endif()


target_link_libraries(big_integer_testing)