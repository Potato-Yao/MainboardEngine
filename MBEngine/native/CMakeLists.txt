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
    find_package(PkgConfig REQUIRED)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(WAYLAND wayland-client)
        pkg_check_modules(WAYLAND_PROTOCOLS wayland-protocols)
    endif ()

    # Generate Wayland protocol files
    if (WAYLAND_FOUND)
        find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)

        # Get wayland-protocols directory
        execute_process(
            COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=pkgdatadir wayland-protocols
            OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        message(STATUS "Wayland protocols directory: ${WAYLAND_PROTOCOLS_DIR}")
        message(STATUS "Wayland scanner: ${WAYLAND_SCANNER}")

        # XDG Shell protocol
        set(XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
        set(XDG_SHELL_CLIENT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h")
        set(XDG_SHELL_PRIVATE_CODE "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.c")

        # Generate client header
        add_custom_command(
            OUTPUT ${XDG_SHELL_CLIENT_HEADER}
            COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_XML} ${XDG_SHELL_CLIENT_HEADER}
            DEPENDS ${XDG_SHELL_XML}
            VERBATIM
            COMMENT "Generating xdg-shell client header"
        )

        # Generate private code
        add_custom_command(
            OUTPUT ${XDG_SHELL_PRIVATE_CODE}
            COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_XML} ${XDG_SHELL_PRIVATE_CODE}
            DEPENDS ${XDG_SHELL_XML}
            VERBATIM
            COMMENT "Generating xdg-shell private code"
        )

        # Create a target for the protocol files
        add_custom_target(wayland_protocols_target
            DEPENDS ${XDG_SHELL_CLIENT_HEADER} ${XDG_SHELL_PRIVATE_CODE}
        )

        set(WAYLAND_PROTOCOL_SOURCES ${XDG_SHELL_PRIVATE_CODE})
        set(WAYLAND_PROTOCOL_HEADERS ${XDG_SHELL_CLIENT_HEADER})

        message(STATUS "Wayland protocol sources configured")
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
        endif ()
    endif ()

    # Fallback: Check for display server environment variables
    if (NOT USE_WAYLAND AND NOT USE_X11)
        if (DEFINED ENV{WAYLAND_DISPLAY})
            set(USE_WAYLAND TRUE)
            message(STATUS "Desktop Environment: Wayland (detected via WAYLAND_DISPLAY)")
        elseif (DEFINED ENV{DISPLAY})
            set(USE_X11 TRUE)
            message(STATUS "Desktop Environment: X11 (detected via DISPLAY)")
        else ()
            # Default to X11 if nothing is detected
            set(USE_X11 TRUE)
            message(STATUS "Desktop Environment: X11 (default)")
        endif ()
    endif ()
endif ()

add_library(mainboard_native SHARED
        platform.cpp
        engine.cpp
)

# Add Wayland protocol sources if available
if (WAYLAND_FOUND AND WAYLAND_PROTOCOL_SOURCES)
    target_sources(mainboard_native PRIVATE ${WAYLAND_PROTOCOL_SOURCES})
    add_dependencies(mainboard_native wayland_protocols_target)
endif()

if (WIN32)
    target_compile_definitions(mainboard_native PRIVATE MAINBOARD_NATIVE_EXPORTS)
endif ()

# Define desktop environment macros
if (USE_X11)
    target_compile_definitions(mainboard_native PRIVATE __ME_USE_X11__)
endif ()

if (USE_WAYLAND)
    target_compile_definitions(mainboard_native PRIVATE __ME_USE_WAYLAND__)
endif ()

target_include_directories(mainboard_native PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mainboard_native
        bgfx
        bimg
        bx
)

# Link Wayland client library if using Wayland
if (USE_WAYLAND AND WAYLAND_FOUND)
    target_link_libraries(mainboard_native wayland-client)
endif ()

# Create test executable
add_executable(test_native
        tests/test.cpp
        tests/win32_window_test.h
        tests/bgfx_test.h
        tests/wayland_window_test.h
        tests/window_test.h
        tests/engine_render_test.h)

# Add Wayland protocol sources if available
if (WAYLAND_FOUND AND WAYLAND_PROTOCOL_SOURCES)
    target_sources(test_native PRIVATE ${WAYLAND_PROTOCOL_SOURCES})
    add_dependencies(test_native wayland_protocols_target)
    target_link_libraries(test_native wayland-client)
endif()

target_include_directories(test_native PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(test_native
        mainboard_native
        bgfx
        bimg
        bx)
