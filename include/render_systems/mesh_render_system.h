#pragma once

#include "render_systems/render_system.h"
#include "renderer/command.h"
#include "renderer/pipeline.h"
#include "renderer/mesh.h"

class MeshRenderSystem : public RenderSystem {
public:
    void render(Command* cmd);

    void initialize(Renderer* renderer);
    void cleanup();

    void add_renderable(GPUMesh* renderable);
    void update_push_constants(GPUDrawPushConstants* push_constants);

    Pipeline simple_mesh_pipeline;
    std::vector<GPUMesh*> renderables;
    GPUDrawPushConstants* push_constants;
};
