find_package(Vulkan REQUIRED) # Obtains all Vulkan API compiling info

# engine/CMakeLists.txt
project("GraphicsEngine")

file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE ENGINE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx"
)

# Engine as a static library
add_library(GraphicsEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Add engine source files
add_subdirectory(dependencies) # This may change, want to use submodules for dependencies

# Link Vulkan to the engine
target_link_libraries(GraphicsEngine PUBLIC Vulkan::Vulkan)

# Make engine headers available to projects that link against it
target_include_directories(GraphicsEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
)

# Engine-specific compile features
target_compile_features(GraphicsEngine PUBLIC cxx_std_23)

