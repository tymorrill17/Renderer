#include "render_systems/render_system.h"
#include "renderer/command.h"
#include "renderer/pipeline.h"


class MeshRenderSystem : public RenderSystem {
public:
    void render(Command* cmd);

    void initialize(Renderer* renderer);

    Pipeline simple_mesh_pipeline;
};
