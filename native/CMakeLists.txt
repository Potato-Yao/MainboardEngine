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

# Detect desktop environment on Linux
if (UNIX AND NOT APPLE)
    # Check for Wayland
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(WAYLAND wayland-client)
    endif()

    # Check XDG_SESSION_TYPE environment variable at configure time
    if (DEFINED ENV{XDG_SESSION_TYPE})
        set(XDG_SESSION_TYPE $ENV{XDG_SESSION_TYPE})
        message(STATUS "Detected XDG_SESSION_TYPE: ${XDG_SESSION_TYPE}")

        if (XDG_SESSION_TYPE STREQUAL "wayland")
            set(USE_WAYLAND TRUE)
            message(STATUS "Desktop Environment: Wayland")
        elseif (XDG_SESSION_TYPE STREQUAL "x11")
            set(USE_X11 TRUE)
            message(STATUS "Desktop Environment: X11")
        endif()
    endif()

    # Fallback: Check for display server environment variables
    if (NOT USE_WAYLAND AND NOT USE_X11)
        if (DEFINED ENV{WAYLAND_DISPLAY})
            set(USE_WAYLAND TRUE)
            message(STATUS "Desktop Environment: Wayland (detected via WAYLAND_DISPLAY)")
        elseif (DEFINED ENV{DISPLAY})
            set(USE_X11 TRUE)
            message(STATUS "Desktop Environment: X11 (detected via DISPLAY)")
        else()
            # Default to X11 if nothing is detected
            set(USE_X11 TRUE)
            message(STATUS "Desktop Environment: X11 (default)")
        endif()
    endif()
endif()

add_library(mainboard_native SHARED
        platform.cpp
        engine.cpp
)

if (WIN32)
    target_compile_definitions(mainboard_native PRIVATE MAINBOARD_NATIVE_EXPORTS)
endif ()

# Define desktop environment macros
if (USE_X11)
    target_compile_definitions(mainboard_native PRIVATE __ME_USE_X11__)
#    target_compile_definitions(test_native PRIVATE ME_USE_X11)
endif()

if (USE_WAYLAND)
    target_compile_definitions(mainboard_native PRIVATE __ME_USE_WAYLAND__)
#    target_compile_definitions(test_native PRIVATE ME_USE_WAYLAND)
endif()

target_include_directories(mainboard_native PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
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
target_include_directories(test_native PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
