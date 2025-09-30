#pragma once

#include "render_systems/render_system.h"
#include "renderer/descriptor.h"
#include "renderer/renderer.h"
#include "utility/gui.h"

class GuiRenderSystem : public RenderSystem {
public:
	GuiRenderSystem(Renderer& renderer, Window& window); // Forces there to be a gui object before creating a gui render system
	~GuiRenderSystem();

	void render(Command& cmd);

	void getNewFrame();

	void endFrame();

private:
	Window& _window;
	DescriptorPool _descriptorPool;
};
