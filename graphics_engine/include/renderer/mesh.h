#include "glm/glm.hpp"
#include "renderer/renderer.h"
#include "renderer/buffer.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <span>


// struct MeshVertex {
//     glm::vec3 position;
//     float uv_x;
//     glm::vec3 normal;
//     float uv_y;
//     glm::vec4 color;
// };
//
// struct Mesh {
//     Mesh();
//     Mesh(std::span<MeshVertex> vertices, std::span<uint32_t> indices);
//     ~Mesh();
//
//     std::span<MeshVertex> _vertices;
//     std::span<uint32_t> _indices;
//
//     Buffer _vertexBuffer;
//     VkDeviceAddress _vertexBufferAddress;
//     Buffer _indexBuffer;
//
//     void cleanup();
//
// };


// UNRELATED NOTE: MAKE DELETION QUEUES EASY TO USE SO I CAN HAVE MULTIPLE OF THEM.
