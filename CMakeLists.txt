cmake_minimum_required(VERSION 3.20)
project(foreign_struct)

enable_testing()

set(CMAKE_CXX_STANDARD 20)

# directly using ut's CMakeLists.txt produces a large amount of crap, so we reinvent the wheel here
add_library(ut INTERFACE)
target_include_directories(ut INTERFACE ${CMAKE_CURRENT_LIST_DIR}/ext-deps/ut/include)

add_library(cxx-sized-int-literals INTERFACE)
target_include_directories(cxx-sized-int-literals INTERFACE ${CMAKE_CURRENT_LIST_DIR}/ext-deps/cxx-sized-int-literals)

add_library(cxxopts INTERFACE)
target_include_directories(cxxopts INTERFACE ${CMAKE_CURRENT_LIST_DIR}/ext-deps/cxxopts/include)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake")

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

add_subdirectory(test)
