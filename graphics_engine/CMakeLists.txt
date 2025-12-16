project("GraphicsEngine")

file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_library(GraphicsEngine STATIC "${ENGINE_SOURCES}" "${ENGINE_HEADERS}")

target_include_directories(GraphicsEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

add_subdirectory(dependencies)

