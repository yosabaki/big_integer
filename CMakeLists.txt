cmake_minimum_required(VERSION 3.9)
project(.)

set(CMAKE_CXX_STANDARD 17)

include_directories(${big_integer_SOURCE_DIR})

SET(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
SET(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} --coverage")

add_executable(big_integer_testing
        ../big_integer_testing.cpp
        ../big_integer.h
        ../big_integer.cpp
        ../gtest/gtest-all.cc
        ../gtest/gtest.h
        ../gtest/gtest_main.cc)


target_link_libraries(big_integer_testing)