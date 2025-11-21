#include "render_systems/mesh_render_system.h"
#include "renderer/renderer.h"

void MeshRenderSystem::initialize(Renderer* renderer) {
    renderer->pipeline_builder.clear();
}

void MeshRenderSystem::render(Command* cmd) {

}
