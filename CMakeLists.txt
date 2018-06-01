cmake_minimum_required(VERSION 3.5.1)

project(big_integer)
include(gtest.cmake)

set(CMAKE_CXX_STANDARD 17)

include_directories(${big_integer_SOURCE_DIR})

SET(COVERAGE OFF CACHE BOOL "Coverage")

add_executable(big_integer_testing
        ./big_integer_testing.cpp
        ./big_integer.h
        ./big_integer.cpp)

find_package(Threads)
target_link_libraries(big_integer_testing gtest ${CMAKE_THREAD_LIBS_INIT})

target_compile_options(big_integer_testing PRIVATE -std=c++17 -g)

if (COVERAGE)
    target_compile_options(big_integer_testing PRIVATE --coverage)
    set_target_properties(big_integer_testing PROPERTIES LINK_FLAGS --coverage)
endif ()
