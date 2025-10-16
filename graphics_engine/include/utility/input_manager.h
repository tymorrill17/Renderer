#pragma once
#include "utility/window.h"
#include "glm/glm.hpp"
#include "utility/gui.h"
#include <vector>
#include <unordered_map>
#include <iostream>

// Enums describing input events for ease of code reading
enum class InputEvent {
	left_mouse_down, left_mouse_up,
	right_mouse_down, right_mouse_up,
	space_down, space_up,
	right_arrow_down, right_arrow_up
};

class InputManager {
public:
    void initialize(Window* window);

	void process_inputs();
    void update_mouse_position(SDL_Event* e);

	void add_listener(InputEvent input_event, std::function<void()> callback);
    void dispatch_event(InputEvent event);

	Window* window;
	std::unordered_map<InputEvent, std::vector<std::function<void()>>> listeners;
    glm::vec2 mouse_position{ 0.0f, 0.0f };
};

