#pragma once

#include "command.h"

class Renderer;

class RenderSystem {
public:
	virtual void render(Command* cmd) = 0;

};
