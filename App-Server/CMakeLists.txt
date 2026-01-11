# App-Server/CMakeLists.txt

file(GLOB_RECURSE APP_SERVER_SOURCES
    Source/*.h
    Source/*.cpp
)

if(WL_HEADLESS)
    set(APP_SERVER_TARGET App-Server-Headless)
else()
    set(APP_SERVER_TARGET App-Server)
endif()

add_executable(${APP_SERVER_TARGET} ${APP_SERVER_SOURCES})

target_include_directories(${APP_SERVER_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/App-Common/Source
    ${CMAKE_SOURCE_DIR}/Walnut/vendor/glm
    ${CMAKE_SOURCE_DIR}/Walnut/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/Walnut/vendor/yaml-cpp/include
    ${CMAKE_SOURCE_DIR}/Walnut/Walnut/Source
    ${CMAKE_SOURCE_DIR}/Walnut/Walnut-Modules/Walnut-Networking/Source
    ${CMAKE_SOURCE_DIR}/Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/include
)

# Platform-specific includes
if(WL_HEADLESS)
    target_include_directories(${APP_SERVER_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/Walnut/Walnut/Platform/Headless
    )
else()
    target_include_directories(${APP_SERVER_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/Walnut/vendor/imgui
        ${CMAKE_SOURCE_DIR}/Walnut/vendor/glfw/include
        ${CMAKE_SOURCE_DIR}/Walnut/Walnut/Platform/GUI
    )
endif()

target_compile_definitions(${APP_SERVER_TARGET} PRIVATE
    YAML_CPP_STATIC_DEFINE
)

if(WL_HEADLESS)
    target_compile_definitions(${APP_SERVER_TARGET} PRIVATE WL_HEADLESS)
    
    target_link_libraries(${APP_SERVER_TARGET} PRIVATE
        App-Common-Headless
        Walnut-Headless
        Walnut-Networking
        yaml-cpp
    )
else()
    target_link_libraries(${APP_SERVER_TARGET} PRIVATE
        App-Common
        Walnut
        Walnut-Networking
        ImGui
        GLFW
        yaml-cpp
        Vulkan::Vulkan
    )
endif()

# Platform-specific settings
if(APPLE)
    # Set rpath for finding dylibs next to executable
    set_target_properties(${APP_SERVER_TARGET} PROPERTIES
        BUILD_RPATH "@executable_path"
        INSTALL_RPATH "@executable_path"
    )
    
    target_link_libraries(${APP_SERVER_TARGET} PRIVATE
        GameNetworkingSockets
    )
    
    if(NOT WL_HEADLESS)
        target_link_libraries(${APP_SERVER_TARGET} PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreFoundation"
        )
    endif()
    
    # Copy dylibs to output directory
    set(GNS_DYLIB ${CMAKE_SOURCE_DIR}/Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/Macos/libGameNetworkingSockets.dylib)
    
    add_custom_command(TARGET ${APP_SERVER_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GNS_DYLIB}
            $<TARGET_FILE_DIR:${APP_SERVER_TARGET}>/libGameNetworkingSockets.dylib
        COMMENT "Copying runtime libraries..."
    )
    
    # Copy MoltenVK only for GUI builds
    if(NOT WL_HEADLESS)
        set(MOLTENVK_DYLIB ${CMAKE_SOURCE_DIR}/Walnut/vendor/MoltenVK/libMoltenVK.dylib)
        add_custom_command(TARGET ${APP_SERVER_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${MOLTENVK_DYLIB}
                $<TARGET_FILE_DIR:${APP_SERVER_TARGET}>/libvulkan.1.dylib
            COMMENT "Copying MoltenVK..."
        )
    endif()
    
    # Link directory for GameNetworkingSockets
    target_link_directories(${APP_SERVER_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/Macos
    )

elseif(WIN32)
    target_link_libraries(${APP_SERVER_TARGET} PRIVATE Ws2_32)
    
    # Copy DLLs to output directory
    set(GNS_BIN_DIR ${CMAKE_SOURCE_DIR}/Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/Windows/$<IF:$<CONFIG:Debug>,Debug,Release>)
    
    add_custom_command(TARGET ${APP_SERVER_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GNS_BIN_DIR}/GameNetworkingSockets.dll
            $<TARGET_FILE_DIR:${APP_SERVER_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GNS_BIN_DIR}/libcrypto-3-x64.dll
            $<TARGET_FILE_DIR:${APP_SERVER_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GNS_BIN_DIR}/libprotobufd.dll
            $<TARGET_FILE_DIR:${APP_SERVER_TARGET}>
        COMMENT "Copying runtime DLLs..."
    )
endif()

# Output directory structure to match premake
set_target_properties(${APP_SERVER_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${OUTPUT_DIR}/${APP_SERVER_TARGET}
)
