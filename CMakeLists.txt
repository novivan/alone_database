cmake_minimum_required(VERSION 3.10)

project(MyDB)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB SOURCES src/main.cpp)

add_executable(memorydb ${SOURCES})