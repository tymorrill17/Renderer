#pragma once

#include "fwd.hpp"
#include "glm/glm.hpp"
#include "renderer/renderer.h"
#include "renderer/buffer.h"
#include "vulkan/vulkan_core.h"
#include <span>
#include <cstdint>


struct MeshVertex {
    glm::vec3 position;
    float uv_x;
    glm::vec3 normal;
    float uv_y;
    glm::vec4 color;
};

struct GPUDrawPushConstants {
    glm::mat4 world_matrix;
    VkDeviceAddress vertex_buffer_address;
};

class GPUMeshBuffers {
public:
    Buffer vertex_buffer;
    VkDeviceAddress vertex_buffer_address;
    Buffer index_buffer;

    Renderer* renderer;

    void upload_to_buffers(Renderer* renderer, std::span<MeshVertex> vertices, std::span<uint32_t> indices);
    void cleanup();
};

class Mesh {
public:
    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indices;
    GPUMeshBuffers gpu_buffers;

    void upload_to_GPU(Renderer* renderer);
};

namespace PrimitiveShapes {
    Mesh Rectangle(float width, float height);
    Mesh Triangle(float base, float height, float skew);
}
