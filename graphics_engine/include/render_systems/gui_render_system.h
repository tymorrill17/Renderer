#pragma once

#include "render_systems/render_system.h"
#include "renderer/descriptor.h"
#include "renderer/renderer.h"
#include "utility/gui.h"

class GuiRenderSystem : public RenderSystem {
public:
    void initialize(Renderer* renderer);
    void cleanup();

    void start_frame();
    void end_frame();

    void render(Command* cmd);

    DescriptorPool descriptor_pool;
};
