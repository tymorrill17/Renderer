#include "window.h"
#include "logger.h"
#include "GLFW/glfw3.h"
#include "logger.h"
#include <cstdint>
#include <string>

void Window::initialize(uint32_t width, uint32_t height, const std::string& name) {

    window_should_close = false;
    fullscreen = false;
    resized = false;
    pause_rendering = false;

    if (!glfwInit()) {
        Logger::logError("GLFW initialization failed!");
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Allows use of GLFW without an OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE); // Wayland needs this for some reason..
    glfw_window = glfwCreateWindow(
        width,
        height,
        name.c_str(),
        nullptr, // May specify a monitor here to start in fullscreen
        nullptr
    );

	if (!glfw_window) {
        Logger::logError("Window creation failure!");
	}
    Logger::log("Created a window titled \"" + name + "\"");

    glfw_monitor = glfwGetPrimaryMonitor();
    glfw_mode = glfwGetVideoMode(glfw_monitor);

    update_window_info();
}

void Window::cleanup() {
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

void Window::get_required_instance_extensions(std::vector<const char*>* extensions) {
	uint32_t glfw_extension_count;
    const char** glfw_required_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	extensions->assign(glfw_required_extensions, glfw_required_extensions + glfw_extension_count);
}

void Window::update_window_info() {
    glfwGetWindowContentScale(glfw_window, &x_scale, &y_scale);

    int x, y;
    glfwGetWindowSize(glfw_window, &x, &y);
    logical_extent.width  = x;
    logical_extent.height = y;
    aspect_ratio = float(logical_extent.width) / float(logical_extent.height);

    glfwGetFramebufferSize(glfw_window, &x, &y);
    framebuffer_extent.width  = x; // logical_extent.width  * x_scale;
    framebuffer_extent.height = y; // logical_extent.height * y_scale;

    glfwGetWindowPos(glfw_window, &x, &y);
    x_pos = x;
    y_pos = y;
}

void Window::set_fullscreen(bool state) {
    if (state == true) { // Go to fullscreen

        // Save the window size and height for toggling out of fullscreen mode
        glfwGetWindowSize(glfw_window, &x_size_saved, &y_size_saved);
        glfwGetWindowPos(glfw_window, &x_pos_saved, &y_pos_saved);

        // This call sets window to fullscreen
        glfwSetWindowMonitor(glfw_window, glfw_monitor, 0, 0, glfw_mode->width, glfw_mode->height, glfw_mode->refreshRate);

    } else { // Go back to windowed mode
        glfwSetWindowMonitor(glfw_window, nullptr, x_pos_saved, y_pos_saved, x_size_saved, y_size_saved, 0);
    }

    fullscreen = state;
    resized = true;
}
