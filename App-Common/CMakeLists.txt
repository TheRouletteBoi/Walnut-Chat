# App-Common/CMakeLists.txt

file(GLOB_RECURSE APP_COMMON_SOURCES
    Source/*.h
    Source/*.cpp
)

if(WL_HEADLESS)
    set(APP_COMMON_TARGET App-Common-Headless)
else()
    set(APP_COMMON_TARGET App-Common)
endif()

add_library(${APP_COMMON_TARGET} STATIC ${APP_COMMON_SOURCES})

target_include_directories(${APP_COMMON_TARGET} PUBLIC
    Source
    ${CMAKE_SOURCE_DIR}/Walnut/vendor/glm
    ${CMAKE_SOURCE_DIR}/Walnut/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/Walnut/Walnut/Source
    ${CMAKE_SOURCE_DIR}/Walnut/Walnut-Modules/Walnut-Networking/Source
    ${CMAKE_SOURCE_DIR}/Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/include
)

# Add GUI-specific includes only when not headless
if(NOT WL_HEADLESS)
    target_include_directories(${APP_COMMON_TARGET} PUBLIC
        ${CMAKE_SOURCE_DIR}/Walnut/vendor/imgui
        ${CMAKE_SOURCE_DIR}/Walnut/vendor/glfw/include
    )
endif()

if(WL_HEADLESS)
    target_link_libraries(${APP_COMMON_TARGET} PUBLIC
        Walnut-Headless
        Walnut-Networking
    )
    target_compile_definitions(${APP_COMMON_TARGET} PUBLIC WL_HEADLESS)
else()
    target_link_libraries(${APP_COMMON_TARGET} PUBLIC
        Walnut
        Walnut-Networking
    )
endif()

# Set output directory to match premake structure
set_target_properties(${APP_COMMON_TARGET} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${OUTPUT_DIR}/${APP_COMMON_TARGET}
)
