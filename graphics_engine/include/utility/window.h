#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "vulkan/vulkan_core.h"
#include <GLFW/glfw3.h>

class Window {
public:
    void initialize(uint32_t width, uint32_t height, const std::string& name);
    void cleanup();
    void update_window_info();

    void set_fullscreen(bool state);

    static void get_required_instance_extensions(std::vector<const char*>* extensions);

    GLFWwindow*  glfw_window;
    GLFWmonitor* glfw_monitor;
    const GLFWvidmode* glfw_mode;

	VkSurfaceKHR surface;
    float aspect_ratio;

    VkExtent2D logical_extent;
    VkExtent2D framebuffer_extent;
    uint32_t x_pos,   y_pos;
    float    x_scale, y_scale;

    // To save position and size when entering fullscreen
    int x_pos_saved, y_pos_saved;
    int x_size_saved, y_size_saved;

    bool window_should_close;
    bool pause_rendering;
    bool fullscreen;
    bool resized;
};
