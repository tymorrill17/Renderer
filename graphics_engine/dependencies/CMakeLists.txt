find_package(Vulkan REQUIRED)
target_link_libraries(GraphicsEngine PUBLIC Vulkan::Vulkan)

# VMA - header-only library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vma")
    add_subdirectory(vma)
    target_link_libraries(GraphicsEngine PUBLIC GPUOpen::VulkanMemoryAllocator)
    message(STATUS "VMA found and configured")
else()
    message(WARNING "VMA not found")
endif()

# GLM - header-only library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glm")
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
    target_link_libraries(GraphicsEngine PUBLIC glm)
    message(STATUS "GLM found and configured")
else()
    message(WARNING "GLM not found")
endif()

# STB Image - header-only library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stb")
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")
    target_link_libraries(GraphicsEngine PUBLIC stb)
    message(STATUS "STB Image found and configured")
else()
    message(WARNING "STB Image not found")
endif()

# SDL has its own CMakeLists.txt
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sdl")
    add_subdirectory(sdl EXCLUDE_FROM_ALL)
    target_link_libraries(GraphicsEngine PUBLIC SDL3::SDL3)
    message(STATUS "SDL3 found and configured")
else()
    message(WARNING "SDL3 not found")
endif()

# Slang API - pre-compiled library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/slang")
    find_package(slang REQUIRED COMPONENTS)
    target_link_libraries(GraphicsEngine PUBLIC slang::slang)
    if(slang_FOUND)
        message(STATUS "slang found and configured")
    endif()
else()
    message(WARNING "slang not found")
endif()

# ImGui - source and include files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
    # It is recommended not to add imgui as a static or shared library
    target_include_directories(GraphicsEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
    # Collect ImGui source files
    file(GLOB IMGUI_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp"
    )
    target_sources(GraphicsEngine PUBLIC "${IMGUI_SOURCES}")
    message(STATUS "ImGui found and configured")
else()
    message(WARNING "ImGui not found")
endif()

# fastGLTF - has its own CMakeLists.txt
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fastgltf")
    add_subdirectory(fastgltf)
    target_link_libraries(GraphicsEngine PUBLIC fastgltf)
    target_include_directories(GraphicsEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/fastgltf/include")
    message(STATUS "fastgltf found and configured")
else()
    message(WARNING "fastgltf not found")
endif()

message(STATUS "All dependencies configured for GraphicsEngine")
