cmake_minimum_required(VERSION 4.0)
project(native)

set(CMAKE_CXX_STANDARD 17)

add_library(native SHARED library.cpp
        mainboard_engine.h
        platform.cpp
        platform.h
)
target_include_directories(native PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_native tests/Win32WindowTest.cpp)
target_link_libraries(test_native native)
