#include "renderable.h"
#include "renderer.h"
#include <vulkan/vulkan_core.h>

MaterialInstance Material::GLTFMetallicRoughnessDiffuse(
    Renderer *renderer,
    Buffer* global_ubo,
    glm::vec4 color_factors, AllocatedImage* color_image, VkSampler color_sampler,
    glm::vec4 roughness_factors, AllocatedImage* roughness_image, VkSampler roughness_sampler) {

    MaterialInstance new_material;

    renderer->pipeline_builder.clear();
    renderer->descriptor_builder.clear();

    Shader mesh_vertex_shader;
    Shader mesh_pixel_shader;
    mesh_vertex_shader.initialize(&renderer->device, &renderer->shader_manager, VK_SHADER_STAGE_VERTEX_BIT, "vertex_main");
    mesh_pixel_shader.initialize(&renderer->device, &renderer->shader_manager, VK_SHADER_STAGE_FRAGMENT_BIT, "pixel_main");

    new_material.set = renderer->descriptor_builder
        .add_buffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, global_ubo)
        .add_image(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, color_image, color_sampler)
        .add_image(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, roughness_image, roughness_sampler)
        .build();

    VkPushConstantRange push_constant_range{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(GPUDrawPushConstants),
    };

    new_material.pipeline = renderer->pipeline_builder
        .set_shader(mesh_vertex_shader)
        .set_shader(mesh_pixel_shader)
        .add_descriptor(new_material.set.layout);

}

MaterialInstance Material::GLTFMetallicRoughnessTransparent(
    Renderer *renderer,
    Buffer* global_ubo,
    glm::vec4 color_factors, AllocatedImage* color_image, VkSampler color_sampler,
    glm::vec4 roughness_factors, AllocatedImage* roughness_image, VkSampler roughness_sampler) {

}
