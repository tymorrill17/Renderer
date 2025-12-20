#include "render_systems/mesh_render_system.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"
#include "utility/logger.h"

#ifdef SHADER_DIR
static const std::string shader_directory{SHADER_DIR};
#endif

void MeshRenderSystem::initialize(Renderer* renderer) {
    // Start building the mesh render pipeline
    renderer->pipeline_builder.clear();

	Shader basic_vertex_shader;
    basic_vertex_shader.initialize(&renderer->device, &renderer->shader_manager, VK_SHADER_STAGE_VERTEX_BIT, "vertex_main");
	Shader basic_pixel_shader;
    basic_pixel_shader.initialize(&renderer->device, &renderer->shader_manager, VK_SHADER_STAGE_FRAGMENT_BIT, "pixel_main");

    VkPushConstantRange push_constant_range{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(GPUDrawPushConstants),
    };

    simple_mesh_pipeline = renderer->pipeline_builder
        .add_push_constant(push_constant_range)
        .set_shader(basic_vertex_shader)
        .set_shader(basic_pixel_shader)
        .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .set_polygon_mode(VK_POLYGON_MODE_FILL)
        .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
        .set_multisampling(VK_SAMPLE_COUNT_1_BIT)
        .set_blending(false)
        .set_color_attachment_format(renderer->draw_image.format)
        .set_depth_attachment_format(VK_FORMAT_UNDEFINED)
        .build();

    basic_pixel_shader.cleanup();
    basic_vertex_shader.cleanup();
}

void MeshRenderSystem::cleanup() {
    simple_mesh_pipeline.cleanup();
}

void MeshRenderSystem::add_renderable(GPUMesh* renderable) {
    renderables.push_back(renderable);
}

void MeshRenderSystem::update_push_constants(GPUDrawPushConstants* push_constants) {
    this->push_constants = push_constants;
}

void MeshRenderSystem::render(Command* cmd) {
    vkCmdBindPipeline(cmd->buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, simple_mesh_pipeline.handle);
    if (this->push_constants) vkCmdPushConstants(cmd->buffer, simple_mesh_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), push_constants);
    for (auto renderable : this->renderables) {
        vkCmdBindIndexBuffer(cmd->buffer, renderable->index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd->buffer, renderable->index_count, 1, 0, 0, 0);
    }
}
