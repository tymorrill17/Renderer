#pragma once

#include "NonCopyable.h"
#include "renderer/command.h"

class Renderer;

class RenderSystem : public NonCopyable {
public:
	RenderSystem(Renderer& renderer) : _renderer(renderer) {}
	virtual void render(Command& cmd) = 0;

protected:
	Renderer& _renderer;
};
