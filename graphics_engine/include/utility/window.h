#pragma once
#define SDL_MAIN_HANDLED
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "utility/logger.h"
#include <string>
#include <vector>
#include <iostream>
#include "vulkan/vulkan_core.h"

class Window {
public:
    void initialize(uint32_t width, uint32_t height, const std::string& name);
    void cleanup();
    void update_after_resize();

    static void get_required_instance_extensions(std::vector<const char*>* extensions);

	struct SDL_Window* sdl_window;
	VkExtent2D extent;
	VkSurfaceKHR surface;
    float aspect_ratio;
    bool window_should_close;
    bool pause_rendering;
    bool fullscreen;
};
