#pragma once

#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "renderer/buffer.h"
#include "vulkan/vulkan_core.h"
#include <span>
#include <cstdint>

class Renderer;

struct MeshVertex {
    glm::vec3 position;
    float     uv_x;
    glm::vec3 normal;
    float     uv_y;
    glm::vec4 color;
};

struct GPUDrawPushConstants {
    glm::mat4 world_matrix;
    VkDeviceAddress vertex_buffer_address;
};

class GPUMesh {
public:
    Buffer vertex_buffer;
    size_t vertex_count; // How many vertices
    Buffer index_buffer;
    size_t index_count;  // How many indices
    VkDeviceAddress vertex_buffer_address;

    void upload_to_GPU(Renderer* renderer, std::span<MeshVertex> vertices, std::span<uint32_t> indices);
    void cleanup();
};

class PrimitiveMesh {
public:
    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indices;

    // Define sets of vertices and indices from shape attributes
    static PrimitiveMesh Rectangle2D(float width, float height);
    static PrimitiveMesh Triangle2D(float base, float height, float skew);
};
