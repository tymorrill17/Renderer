cmake_minimum_required (VERSION 4.1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    if(MSVC)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

# target_link_libraries() allows use with targets in other directories
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # output compile_commands.json to properly set up clangd for use in neovim (if applicable)

set(CMAKE_CXX_STANDARD 23)

set (PROJECT_NAME "Renderer")
project("${PROJECT_NAME}")

# compile definitions
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin") # Set folder to output executables
add_executable("${PROJECT_NAME}")

# Project-specific include directories
target_include_directories("${PROJECT_NAME}" PUBLIC "${PROJECT_SOURCE_DIR}/include")

# Add project source files
file(GLOB_RECURSE PROJECT_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
target_sources("${PROJECT_NAME}" PRIVATE "${PROJECT_SOURCES}" "${PROJECT_HEADERS}")

add_compile_definitions(
    SHADER_DIR="${PROJECT_SOURCE_DIR}/shaders/"
    ROOT_DIR="${PROJECT_SOURCE_DIR}"
)

add_subdirectory(graphics_engine)
target_link_libraries("${PROJECT_NAME}" PUBLIC GraphicsEngine)

set(ExecutableName "renderer")
set_target_properties("${PROJECT_NAME}" PROPERTIES OUTPUT_NAME "${ExecutableName}")

