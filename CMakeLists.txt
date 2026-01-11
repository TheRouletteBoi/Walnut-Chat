cmake_minimum_required(VERSION 3.20)
project(Walnut-Chat LANGUAGES C CXX OBJC)

# ==============================================================================
# Global Settings
# ==============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories (matches premake structure)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})

# Simplified output dir name to match premake structure
if(APPLE)
    # Detect architecture
    if(CMAKE_OSX_ARCHITECTURES)
        set(ARCH ${CMAKE_OSX_ARCHITECTURES})
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(ARCH "arm64")
    else()
        set(ARCH "x86_64")
    endif()
    set(OUTPUT_DIR "${CMAKE_BUILD_TYPE}-macosx-${ARCH}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${OUTPUT_DIR})
elseif(WIN32)
    set(OUTPUT_DIR "${CMAKE_BUILD_TYPE}-windows-x64")
elseif(UNIX)
    set(OUTPUT_DIR "${CMAKE_BUILD_TYPE}-linux-x64")
endif()

# ==============================================================================
# Build Options
# ==============================================================================
option(WL_HEADLESS "Build headless (no GUI)" OFF)
option(WL_BUILD_CLIENT "Build App-Client" ON)
option(WL_BUILD_SERVER "Build App-Server" ON)

# ==============================================================================
# Find Vulkan (only for GUI builds)
# ==============================================================================
if(NOT WL_HEADLESS)
    find_package(Vulkan REQUIRED)
    message(STATUS "Vulkan found: ${Vulkan_INCLUDE_DIRS}")
endif()

# ==============================================================================
# Platform Definitions
# ==============================================================================
if(APPLE)
    add_compile_definitions(WL_PLATFORM_MACOS)
elseif(WIN32)
    add_compile_definitions(WL_PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(WL_PLATFORM_LINUX)
endif()

# Configuration definitions
add_compile_definitions(
    $<$<CONFIG:Debug>:WL_DEBUG>
    $<$<CONFIG:Release>:WL_RELEASE>
    $<$<CONFIG:Dist>:WL_DIST>
)

if(WL_HEADLESS)
    add_compile_definitions(WL_HEADLESS)
endif()

# ==============================================================================
# Subdirectories
# ==============================================================================

# Walnut and all its dependencies
add_subdirectory(Walnut)

# Application projects
add_subdirectory(App-Common)

if(WL_BUILD_CLIENT)
    add_subdirectory(App-Client)
endif()

if(WL_BUILD_SERVER)
    add_subdirectory(App-Server)
endif()

# ==============================================================================
# Summary
# ==============================================================================
message(STATUS "")
message(STATUS "=== Walnut-Chat Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Headless: ${WL_HEADLESS}")
message(STATUS "Output: build/bin/${OUTPUT_DIR}/")
message(STATUS "")
