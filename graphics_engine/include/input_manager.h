#pragma once
#include "GLFW/glfw3.h"
#include "window.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include "glm/glm.hpp"

// Enums describing input events for ease of code reading
enum class InputEvent {
	MouseLeftDown, MouseLeftUp,
	MouseRightDown, MouseRightUp,
	KeySpaceDown, KeySpaceUp,
	KeyRightArrowDown, KeyRightArrowUp
};

class InputManager {
public:
    void initialize(Window* window);

	void process_inputs();

	void add_listener(InputEvent input_event, std::function<void()> callback);
    void dispatch_event(InputEvent event);

	Window* window;
	std::unordered_map<InputEvent, std::vector<std::function<void()>>> listeners;
    double mouse_pos_x, mouse_pos_y;
};

