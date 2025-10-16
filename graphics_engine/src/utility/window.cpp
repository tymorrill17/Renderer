#include "utility/window.h"
#include "SDL2/SDL_vulkan.h"
#include "utility/logger.h"
#include <cstdint>
#include <string>

void Window::initialize(uint32_t width, uint32_t height, const std::string& name) {

    extent.width  = width;
    extent.height = height;

    window_should_close = false;
    fullscreen = false;
    pause_rendering = false;

    aspect_ratio = float(extent.width) / float(extent.height);

	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Create a window compatible with Vulkan surfaces
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	sdl_window = SDL_CreateWindow(
		name.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		extent.width,
		extent.height,
		window_flags
	);

	if (sdl_window) {
        Logger::log("Created a window titled \"" + name + "\" of size " + std::to_string(extent.width) + "by" + std::to_string(extent.height));
	} else {
        Logger::logError("Window creation failure!");
	}
}

void Window::cleanup() {
	SDL_DestroyWindow(sdl_window);
}

void Window::get_required_instance_extensions(std::vector<const char*>* extensions) {
	uint32_t sdl_required_extension_count = 0;
	SDL_Vulkan_GetInstanceExtensions(nullptr, &sdl_required_extension_count, nullptr);
	std::vector<const char*> sdl_required_extensions(sdl_required_extension_count);
	SDL_Vulkan_GetInstanceExtensions(nullptr, &sdl_required_extension_count, sdl_required_extensions.data());

	extensions->assign(sdl_required_extensions.data(), sdl_required_extensions.data() + sdl_required_extension_count);
}

void Window::update_after_resize() {
	int width, height;
	SDL_GetWindowSize(sdl_window, &width, &height);
	extent.width = width;
	extent.height = height;
    aspect_ratio = float(extent.width) / float(extent.height);
}

