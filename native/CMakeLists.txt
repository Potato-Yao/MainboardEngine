cmake_minimum_required(VERSION 3.20)
project(mainboard_engine)

set(CMAKE_CXX_STANDARD 17)

# Put all MEbuild outputs into the `MEbuild` folder at project root
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MEbuild")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})   # .lib (import or static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})   # .dll (on Windows)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})   # executables (like tests)

add_library(mainboard_native SHARED
        platform.cpp
)

if (WIN32)
    target_compile_definitions(mainboard_native PRIVATE MAINBOARD_NATIVE_EXPORTS)
endif ()

target_include_directories(mainboard_native PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_executable(test_native tests/Win32WindowTest.cpp)
target_link_libraries(test_native mainboard_native)
target_include_directories(test_native PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)