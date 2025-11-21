cmake_minimum_required (VERSION 4.1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    if(MSVC)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

# target_link_libraries() allows use with targets in other directories
# (to use compile_project_shaders in any CMakeLists file)
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

# Sets C++ Standard to 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# output compile_commands.json to properly set up clangd for use in neovim (if applicable)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# STEP 1) ENTER THE NAME OF THE PROJECT
set (PROJECT_NAME "Renderer")
project(${PROJECT_NAME})

add_compile_definitions(ROOT_DIR="${PROJECT_SOURCE_DIR}")
add_compile_definitions(SHADER_DIR="${PROJECT_SOURCE_DIR}/shaders/")

# set executable output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

# STEP 2) Create and define executable
# Create the executable
add_executable(${PROJECT_NAME})

# Project-specific include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/include/"
)

# Add project source files
file(GLOB_RECURSE PROJECT_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

# STEP 3) Add any subtree directories
add_subdirectory(graphics_engine) # Contains all third-party libraries

# Link against the engine
target_link_libraries(${PROJECT_NAME} PRIVATE GraphicsEngine)

# Set output name
set(ExecutableName "renderer")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${ExecutableName})

