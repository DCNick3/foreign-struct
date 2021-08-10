cmake_minimum_required(VERSION 3.20)
project(foreign_struct)

set(CMAKE_CXX_STANDARD 20)

add_library(foreign_struct
        src/foreign/library.cpp
        src/foreign/enum.h
        src/foreign/integer.h
        src/foreign/struct.h
        src/foreign/union.h
        src/foreign/util.h)
target_include_directories(foreign_struct PUBLIC src)

add_executable(example example/main.cpp)
target_link_libraries(example foreign_struct)

set(CMAKE_CXX_FLAGS -fconcepts-diagnostics-depth=10)
