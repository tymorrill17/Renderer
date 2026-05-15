#pragma once
#include <cstdint>
#include "buffer.h"
#include "descriptor.h"
#include "glm/glm.hpp"
#include "image.h"
#include "pipeline.h"
#include "vulkan/vulkan.h"

enum class MaterialType : uint8_t {
    DiffuseColor,
    Transparent
};

struct MaterialInstance {
    Pipeline pipeline;
    DescriptorSet set;
    MaterialType type;
};

struct RenderObject {
    uint32_t index_count;
    uint32_t first_index;
    Buffer* index_buffer;

    MaterialInstance* material;

    glm::mat4 transform;
    VkDeviceAddress vertex_buffer_address;
};

class IRenderable {
    virtual void draw(const glm::mat4& top_matrix, DrawContext& context) = 0;
};

namespace Material {

    MaterialInstance GLTFMetallicRoughnessDiffuse(
        Renderer* renderer,
        Buffer* global_ubo,
        glm::vec4 color_factors, AllocatedImage* color_image, VkSampler color_sampler,
        glm::vec4 roughness_factors, AllocatedImage* roughness_image, VkSampler roughness_sampler
    );

    MaterialInstance GLTFMetallicRoughnessTransparent(
        Renderer* renderer,
        Buffer* global_ubo,
        glm::vec4 color_factors, AllocatedImage* color_image, VkSampler color_sampler,
        glm::vec4 roughness_factors, AllocatedImage* roughness_image, VkSampler roughness_sampler
    );
};

