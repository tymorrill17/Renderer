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

class GPUMeshBuffers {
    Buffer vertex_buffer;
    VkDeviceAddress vertex_buffer_address;
    Buffer index_buffer;

    Renderer* renderer;

    void upload_to_buffers(Renderer* renderer, std::span<MeshVertex> vertices, std::span<uint32_t> indices);
    void cleanup();
};

