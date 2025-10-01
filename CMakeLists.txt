cmake_minimum_required (VERSION 4.1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# target_link_libraries() allows use with targets in other directories
# (to use compile_project_shaders in any CMakeLists file)
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# output compile_commands.json to properly set up clangd for use in neovim (if applicable)
add_compile_definitions(ROOT_DIR="${CMAKE_SOURCE_DIR}")
add_compile_definitions(SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders/")

# Sets C++ Standard to 23
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set(CMAKE_CXX_STANDARD 23)
endif()

# STEP 1) ENTER THE NAME OF THE PROJECT
set (PROJECT_NAME "Renderer")
project(${PROJECT_NAME})

# set executable output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# STEP 2) Add any subtree directories

add_subdirectory(graphics_engine) # Contains all third-party libraries

# STEP 3) Create and define executable

# Create the executable
add_executable(${PROJECT_NAME})

# Add project source files
file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

# Link against the engine
target_link_libraries(${PROJECT_NAME} PRIVATE GraphicsEngine)

# Project-specific include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Optional: Set different output name
set(ExecutableName "renderer")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${ExecutableName})

