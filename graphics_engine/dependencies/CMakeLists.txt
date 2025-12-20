find_package(Vulkan REQUIRED)
target_link_libraries(GraphicsEngine PUBLIC Vulkan::Vulkan)
find_package(slang REQUIRED)
target_link_libraries(GraphicsEngine PUBLIC slang::slang)

set(VMA_GIT_REPO https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)
set(GLM_GIT_REPO https://github.com/g-truc/glm.git)
set(STB_GIT_REPO https://github.com/nothings/stb.git)
set(SDL_GIT_REPO https://github.com/libsdl-org/SDL.git)
set(IMGUI_GIT_REPO https://github.com/ocornut/imgui.git)
set(FASTGLTF_GIT_REPO https://github.com/spnda/fastgltf.git)

function(git_clone_repo REPO_URL CLONE_DIR_NAME)
    message(STATUS "Cloning ${REPO_URL}")
    execute_process(
        COMMAND git clone "${REPO_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/${CLONE_DIR_NAME}"
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "git clone failed: ${REPO_URL}")
    endif()
endfunction()

# VMA - header-only library
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vma")
    message(STATUS "VMA not found")
    git_clone_repo("${VMA_GIT_REPO}" "vma")
endif()
add_subdirectory(vma)
target_link_libraries(GraphicsEngine PUBLIC GPUOpen::VulkanMemoryAllocator)
message(STATUS "VMA found and configured")

# GLM - header-only library
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glm")
    message(STATUS "GLM not found")
    git_clone_repo("${GLM_GIT_REPO}" "glm")
endif()
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
target_link_libraries(GraphicsEngine PUBLIC glm)
message(STATUS "GLM found and configured")

# STB Image - header-only library
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stb")
    message(STATUS "STB Image not found")
    git_clone_repo("${STB_GIT_REPO}" "stb")
endif()
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")
target_link_libraries(GraphicsEngine PUBLIC stb)
message(STATUS "STB Image found and configured")

# SDL has its own CMakeLists.txt
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sdl")
    message(STATUS "SDL3 not found")
    git_clone_repo("${SDL_GIT_REPO}" "sdl")
endif()
add_subdirectory(sdl EXCLUDE_FROM_ALL)
target_link_libraries(GraphicsEngine PUBLIC SDL3::SDL3)
message(STATUS "SDL3 found and configured")

# ImGui - source and include files
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
    message(STATUS "ImGui not found")
    git_clone_repo("${IMGUI_GIT_REPO}" "imgui")
endif()
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

# fastGLTF - has its own CMakeLists.txt
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fastgltf")
    message(STATUS "fastgltf not found")
    git_clone_repo("${FASTGLTF_GIT_REPO}" "fastgltf")
endif()
add_subdirectory(fastgltf)
target_link_libraries(GraphicsEngine PUBLIC fastgltf)
target_include_directories(GraphicsEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/fastgltf/include")
message(STATUS "fastgltf found and configured")

# # Slang API - pre-compiled library
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/slang")
#     find_package(slang REQUIRED COMPONENTS)
#     target_link_libraries(GraphicsEngine PUBLIC slang::slang)
#     if(slang_FOUND)
#         message(STATUS "slang found and configured")
#     endif()
# else()
#     message(STATUS "slang not found")
# endif()

message(STATUS "All dependencies configured for GraphicsEngine")
