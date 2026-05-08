#include "utility/window.h"
#include "SDL3/SDL_video.h"
#include "utility/logger.h"
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <string>

void Window::initialize(uint32_t width, uint32_t height, const std::string& name) {

    extent.width  = width;
    extent.height = height;

    window_should_close = false;
    fullscreen = false;
    resized = false;
    pause_rendering = false;

    aspect_ratio = float(extent.width) / float(extent.height);

    // For linux wayland systems, need to set this variable to fix scaling issues when using highdpi with desktop zoom
    #ifdef __linux__
        setenv("SDL_VIDEO_WAYLAND_SCALE_TO_DISPLAY", "1", 1);
    #endif

	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Create a window compatible with Vulkan surfaces
	SDL_WindowFlags window_flags = (SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_MOUSE_RELATIVE_MODE);
	sdl_window = SDL_CreateWindow(
		name.c_str(),
		extent.width,
		extent.height,
		window_flags
	);

	if (sdl_window) {
        Logger::log("Created a window titled \"" + name + "\" of size " + std::to_string(extent.width) + "by" + std::to_string(extent.height));
	} else {
        Logger::logError("Window creation failure!");
	}

    float window_scale = SDL_GetWindowDisplayScale(sdl_window);
    Logger::log("Window Scale: " + std::to_string(window_scale));
}

void Window::cleanup() {
	SDL_DestroyWindow(sdl_window);
}

void Window::get_required_instance_extensions(std::vector<const char*>* extensions) {
	uint32_t sdl_required_extension_count = 0;
	SDL_Vulkan_GetInstanceExtensions(&sdl_required_extension_count);
	const char* const* sdl_required_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_required_extension_count);

	extensions->assign(sdl_required_extensions, sdl_required_extensions + sdl_required_extension_count);
}

void Window::update_after_resize() {
	int width, height;
	SDL_GetWindowSize(sdl_window, &width, &height);
	extent.width = width;
	extent.height = height;
    aspect_ratio = float(extent.width) / float(extent.height);
}

void Window::set_fullscreen(bool state) {
    SDL_SetWindowFullscreen(sdl_window, state);
    fullscreen = state;
    resized = true;
}
