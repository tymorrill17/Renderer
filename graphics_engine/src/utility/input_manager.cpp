#include "utility/input_manager.h"
#include "GLFW/glfw3.h"

void input_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Get our InputManager struct
    InputManager* input_manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    switch (action) {
    case GLFW_PRESS:
        switch (key) {
            case GLFW_KEY_F11:
                // Toggle fullscreen
                input_manager->window->fullscreen ? input_manager->window->set_fullscreen(false) : input_manager->window->set_fullscreen(true);
                break;
            case GLFW_KEY_SPACE:
                input_manager->dispatch_event(InputEvent::space_down);
                break;
            default:
                break;
        }
        break;
    case GLFW_RELEASE:
        switch (key) {
            default:
                break;
        }
        break;
    default:
        break;
    }
}

void mouse_position_callback(GLFWwindow* window, double x_pos, double y_pos) {
    // Get our InputManager struct
    InputManager* input_manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    input_manager->mouse_pos_x = x_pos;
    input_manager->mouse_pos_y = y_pos;
}

void window_minimized_callback(GLFWwindow* window, int minimized) {
    // Get our InputManager struct
    InputManager* input_manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    if (minimized) {
        input_manager->window->pause_rendering = true;
    } else { // Window was restored
        input_manager->window->pause_rendering = false;
        input_manager->window->resized = true;
    }
}

void window_maximized_callback(GLFWwindow* window, int maximized) {
    // Get our InputManager struct
    InputManager* input_manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    input_manager->window->resized = true;
}

void window_close_callback(GLFWwindow* window) {
    // Get our InputManager struct
    InputManager* input_manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    input_manager->window->window_should_close = true;
}

void InputManager::initialize(Window* window) {
    this->window = window;

    // Allows access to a user struct from just the glfw window handle (used for input handling)
    glfwSetWindowUserPointer(window->glfw_window, this);

    glfwSetKeyCallback(window->glfw_window, input_key_callback);
    glfwSetCursorPosCallback(window->glfw_window, mouse_position_callback);
    glfwSetWindowIconifyCallback(window->glfw_window, window_minimized_callback);
    glfwSetWindowMaximizeCallback(window->glfw_window, window_maximized_callback);
    glfwSetWindowCloseCallback(window->glfw_window, window_close_callback);
}

void InputManager::process_inputs() {
    glfwPollEvents();
}

void InputManager::add_listener(InputEvent input_event, std::function<void()> callback) {
    listeners[input_event].push_back(std::move(callback));
}

void InputManager::dispatch_event(InputEvent event) {
	for (auto& callback : listeners[event]) {
		callback();
	}
}
