#include "utility/window.h"
#include "SDL2/SDL_vulkan.h"
#include "utility/logger.h"

Window::Window(glm::ivec2 dimensions, const std::string& name) :
	_windowExtent{ static_cast<uint32_t>(dimensions.x), static_cast<uint32_t>(dimensions.y) },
	_name(name),
    _instance(VK_NULL_HANDLE),
	_surface(VK_NULL_HANDLE),
	_windowShouldClose(false),
	_pauseRendering(false),
	_isFullscreen(false) {

	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Create a window compatible with Vulkan surfaces
	SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	struct SDL_Window* window = SDL_CreateWindow(
		name.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		windowFlags
	);

    _aspectRatio = float(_windowExtent.width) / float(_windowExtent.height);

	if (window) {
        std::cout << "Created a window titled \"" << name << "\" of size " << _windowExtent.width << "x" << _windowExtent.height << "." << std::endl;
	}
	else {
        Logger::logError("Window creation failure!");
	}

	_window = window;
}

Window::~Window() {
	SDL_DestroyWindow(_window);
}

void Window::getRequiredInstanceExtensions(std::vector<const char*>& extensions) {
	uint32_t sdlRequiredExtensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(nullptr, &sdlRequiredExtensionCount, nullptr);
	std::vector<const char*> sdlRequiredExtensions(sdlRequiredExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(nullptr, &sdlRequiredExtensionCount, sdlRequiredExtensions.data());

	extensions.assign(sdlRequiredExtensions.data(), sdlRequiredExtensions.data() + sdlRequiredExtensionCount);
}

void Window::updateSize() {
	int width, height;
	SDL_GetWindowSize(_window, &width, &height);
	_windowExtent.width = width;
	_windowExtent.height = height;
    _aspectRatio = float(_windowExtent.width) / float(_windowExtent.height);
}

VkSurfaceKHR Window::createSurface(VkInstance instance) {

	_instance = instance;

	if (!SDL_Vulkan_CreateSurface(_window, instance, &_surface)) {
        Logger::logError("Failed to create window surface!");
	}

	std::cout << "Created SDL window surface for Vulkan" << std::endl;

    return _surface;
}
