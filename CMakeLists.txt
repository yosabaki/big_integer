cmake_minimum_required(VERSION 3.5.1)

project(big_integer)

set(CMAKE_CXX_STANDARD 17)

include_directories(${big_integer_SOURCE_DIR})

add_executable(big_integer_testing
        ./big_integer_testing.cpp
        ./big_integer.h ./big_integer.cpp
        ./my_vector.cpp ./my_vector.h
        ./gtest/gtest-all.cc
        ./gtest/gtest.h
        ./gtest/gtest_main.cc)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -pedantic -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ")
  set(CMAKE_CXX_FLAGS_RELEASE " ${CMAKE_CXX_FLAGS_RELEASE} ")

endif()

target_link_libraries(big_integer_testing -lpthread -lgmp -lgmpxx)