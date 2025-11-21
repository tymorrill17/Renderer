# Find Vulkan (should already be found in parent, but doesn't hurt to ensure)
find_package(Vulkan REQUIRED)

# VMA (Vulkan Memory Allocator) - header-only library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vma")
    add_library(vma INTERFACE)
    target_include_directories(vma INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/vma"
    )
    target_link_libraries(GraphicsEngine PUBLIC vma)
    message(STATUS "VMA found and configured")
elseif(EXISTS "$ENV{VULKAN_SDK}/Include/vma")
    add_library(vma INTERFACE)
    target_include_directories(vma INTERFACE
        "$ENV{VULKAN_SDK}/Include/vma"
    )
    target_link_libraries(GraphicsEngine PUBLIC vma)
    message(STATUS "VMA found in Vulkan SDK and configured")
else()
    message(WARNING "VMA not found in dependencies or Vulkan SDK")
endif()

# GLM (OpenGL Mathematics) - header-only library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glm")
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
    target_link_libraries(GraphicsEngine PUBLIC glm)
    message(STATUS "GLM found and configured")
elseif(EXISTS "$ENV{VULKAN_SDK}/Include/glm")
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE "$ENV{VULKAN_SDK}/Include/glm")
    target_link_libraries(GraphicsEngine PUBLIC glm)
    message(STATUS "GLM found in Vulkan SDK and configured")
else()
    # Try to find system GLM
    find_package(glm QUIET)
    if(glm_FOUND)
        target_link_libraries(GraphicsEngine PUBLIC glm::glm)
        message(STATUS "System GLM found and configured")
    else()
        message(WARNING "GLM not found in dependencies, system, or Vulkan SDK")
    endif()
endif()

# SDL2 - requires both headers and libraries
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2")
    # Using bundled SDL2
    add_library(sdl2 INTERFACE)
    target_include_directories(sdl2 SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/include")

    # Platform-specific library paths
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64-bit
            target_link_directories(sdl2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/x64")
        else()
            # 32-bit
            target_link_directories(sdl2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/x86")
        endif()
        target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
    elseif(APPLE)
        target_link_directories(sdl2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/macos")
        target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
    else()
        # Linux
        target_link_directories(sdl2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/linux")
        target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
    endif()

    target_link_libraries(GraphicsEngine PUBLIC sdl2)
    message(STATUS "Bundled SDL2 found and configured")
else()
    # Try to find system SDL2
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        target_link_libraries(GraphicsEngine PUBLIC SDL2::SDL2 SDL2::SDL2main)
        message(STATUS "System SDL2 found and configured")
    else()
        # Fallback to Vulkan SDK (if you have SDL2 there)
        if(EXISTS "$ENV{VULKAN_SDK}/Include/SDL2")
            add_library(sdl2 INTERFACE)
            target_include_directories(sdl2 SYSTEM INTERFACE $ENV{VULKAN_SDK}/Include/SDL2)
            target_link_directories(sdl2 INTERFACE $ENV{VULKAN_SDK}/Lib)
            target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
            target_link_libraries(GraphicsEngine PUBLIC sdl2)
            message(STATUS "SDL2 found in Vulkan SDK and configured")
        else()
            message(WARNING "SDL2 not found in dependencies, system, or Vulkan SDK")
        endif()
    endif()
endif()

# Slang API - Requires headers and libraries
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/slang")
    add_library(slang INTERFACE)
    target_include_directories(slang INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/slang/include")
    target_link_directories(slang INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/slang/lib")
    target_link_libraries(GraphicsEngine PUBLIC slang.lib)
    message(STATUS "slang found in dependencies and configured")
elseif(EXISTS "$ENV{VULKAN_SDK}/Include/slang")
    add_library(slang INTERFACE)
    target_include_directories(slang INTERFACE $ENV{VULKAN_SDK}/Include/slang)
    target_link_directories(slang INTERFACE $ENV{VULKAN_SDK}/Lib)
    target_link_libraries(GraphicsEngine PUBLIC slang.lib)
    message(STATUS "slang found in Vulkan SDK and configured")
else() # Portable dependencies not supported yet
    message(WARNING "slang not found in Vulkan SDK or dependencies. System installs not supported yet.")
endif()

# ImGui - static library (requires compilation)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
    add_library(imgui STATIC)
    target_include_directories(imgui SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

    # Collect ImGui source files
    file(GLOB IMGUI_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp"
    )

    file(GLOB IMGUI_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.h"
    )

    target_sources(imgui PRIVATE ${IMGUI_SOURCES} ${IMGUI_HEADERS})
    target_link_libraries(imgui PUBLIC Vulkan::Vulkan)

    # Link SDL2 if available
    if(TARGET sdl2)
        target_link_libraries(imgui PUBLIC sdl2)
    endif()

    target_link_libraries(GraphicsEngine PUBLIC imgui)
    message(STATUS "ImGui found and configured")
else()
    message(WARNING "ImGui not found at ${CMAKE_CURRENT_SOURCE_DIR}/imgui")
endif()

# STB Image - header-only library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stb_image")
    add_library(stb_image INTERFACE)
    target_include_directories(stb_image INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb_image")
    target_link_libraries(GraphicsEngine PUBLIC stb_image)
    message(STATUS "STB Image found and configured")
else()
    message(WARNING "STB Image not found at ${CMAKE_CURRENT_SOURCE_DIR}/stb_image")
endif()

# Link Vulkan to the engine (if not already done in main engine CMakeLists)
target_link_libraries(GraphicsEngine PUBLIC Vulkan::Vulkan)

message(STATUS "All dependencies configured for GraphicsEngine")
