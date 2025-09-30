#pragma once
#include "utility/window.h"
#include "utility/gui.h"
#include "NonCopyable.h"
#include <vector>
#include <unordered_map>
#include <iostream>

// Enums describing input events for ease of code reading
enum class InputEvent {
	leftMouseDown, leftMouseUp,
	rightMouseDown, rightMouseUp,
	spacebarDown, spacebarUp,
	rightArrowDown, rightArrowUp
};

class InputManager : public NonCopyable {
public:
	InputManager(Window& window);

	// @brief Process SDL inputs and trigger input events
	void processInputs();

    // @brief add a callback function to trigger when the corresponding input event is triggered. This is subject to change when I redo the input manager
	void addListener(InputEvent inputEvent, std::function<void()> callback);

	glm::vec2 mousePosition() { return _mousePosition; }

private:
	Window& _window;
	std::unordered_map<InputEvent, std::vector<std::function<void()>>> _listeners;
    glm::vec2 _mousePosition{ 0.0f, 0.0f };

    // Calls the associated callback functions to the given InputEvent
	void dispatchEvent(InputEvent event);

    // Updates internal mouse position. Called when a mouse-related SDL event is triggered
	void updateMousePosition(SDL_Event* e);
};

