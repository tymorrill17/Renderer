#include "render_systems/mesh_render_system.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"
#include "utility/logger.h"
#include <cstddef>

#ifdef SHADER_DIR
static const std::string shader_directory{SHADER_DIR};
#endif

void MeshRenderSystem::initialize(Renderer* renderer, std::vector<DescriptorSet> descriptor_sets) {
    // Start building the mesh render pipeline
    renderer->pipeline_builder.clear();
    this->descriptor_sets = descriptor_sets;

    contiguous_sets.reserve(descriptor_sets.size());
    for (auto& set : descriptor_sets) {
        contiguous_sets.push_back(set.handle);
    }

	Shader basic_vertex_shader;
    basic_vertex_shader.initialize(&renderer->device, &renderer->shader_manager, VK_SHADER_STAGE_VERTEX_BIT, "vertex_main2");
	Shader basic_pixel_shader;
    basic_pixel_shader.initialize(&renderer->device, &renderer->shader_manager, VK_SHADER_STAGE_FRAGMENT_BIT, "pixel_main");

    VkPushConstantRange push_constant_range{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(GPUDrawPushConstants),
    };

    simple_mesh_pipeline = renderer->pipeline_builder
     //   .add_push_constant(push_constant_range)
        .set_shader(basic_vertex_shader)
        .set_shader(basic_pixel_shader)
        .add_vertex_binding_description(PipelineBuilder::vertex_input_binding_description(0, sizeof(MeshVertex), VK_VERTEX_INPUT_RATE_VERTEX))
        .add_vertex_attribute_description(PipelineBuilder::vertex_input_attribute_description(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MeshVertex, position)))
        .add_vertex_attribute_description(PipelineBuilder::vertex_input_attribute_description(0, 1, VK_FORMAT_R32_SFLOAT, offsetof(MeshVertex, uv_x)))
        .add_vertex_attribute_description(PipelineBuilder::vertex_input_attribute_description(0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MeshVertex, normal)))
        .add_vertex_attribute_description(PipelineBuilder::vertex_input_attribute_description(0, 3, VK_FORMAT_R32_SFLOAT, offsetof(MeshVertex, uv_y)))
        .add_vertex_attribute_description(PipelineBuilder::vertex_input_attribute_description(0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshVertex, color)))
        .add_descriptor(descriptor_sets[0].layout)
        .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .set_polygon_mode(VK_POLYGON_MODE_FILL)
        .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
        .set_multisampling(VK_SAMPLE_COUNT_1_BIT)
        .set_blending(false)
        .set_color_attachment_format(renderer->draw_image.format)
        .set_depth_attachment_format(renderer->depth_image.format)
        .set_depth_test(VK_COMPARE_OP_GREATER_OR_EQUAL)
        .build();

    basic_pixel_shader.cleanup();
    basic_vertex_shader.cleanup();
}

void MeshRenderSystem::cleanup() {
    simple_mesh_pipeline.cleanup();
}

void MeshRenderSystem::add_renderable(std::shared_ptr<MeshAsset> renderable) {
    renderables.push_back(renderable);
}

void MeshRenderSystem::update_push_constants(GPUDrawPushConstants* push_constants) {
    this->push_constants = push_constants;
}

void MeshRenderSystem::render(Command* cmd) {
    vkCmdBindPipeline(cmd->buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, simple_mesh_pipeline.handle);
    vkCmdBindDescriptorSets(cmd->buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, simple_mesh_pipeline.layout, 0, descriptor_sets.size(), contiguous_sets.data(), 0, nullptr);
    //if (this->push_constants) vkCmdPushConstants(cmd->buffer, simple_mesh_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), push_constants);
    for (auto renderable : this->renderables) {
        VkDeviceSize offsets{0};
        vkCmdBindVertexBuffers(cmd->buffer, 0, 1, &renderable->GPU_mesh.vertex_buffer.handle, &offsets);
        vkCmdBindIndexBuffer(cmd->buffer, renderable->GPU_mesh.index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd->buffer, renderable->surfaces[0].count, 1, renderable->surfaces[0].index, 0, 0);
    }
}
