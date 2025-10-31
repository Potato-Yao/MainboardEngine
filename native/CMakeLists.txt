cmake_minimum_required(VERSION 3.20)
project(mainboard_engine)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(third_party/bgfx.cmake)

# Put all MEbuild outputs into the `MEbuild` folder at project root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")

# For DLLs (.dll)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")

# For import/static libs (.lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")

# BUILD the library on windows(using developer powershell prompt 2022):
# get into build folder, which is cd build
# cmake .. -G "Visual Studio 17 2022"
# cmake --build . --config Release
# the dll and .lib will be in MEbuild/Release folder

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

target_link_libraries(mainboard_native
        bgfx
        bimg
        bx
)

add_executable(test_native tests/test.cpp
     tests/win32_window_test.h
     tests/bgfx_test.h)
target_include_directories(test_native PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(test_native
    mainboard_native
    bgfx
    bimg
    bx
)
target_include_directories(test_native PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
