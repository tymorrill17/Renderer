#pragma once
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "utility/logger.h"
#include <glm/vec2.hpp>
#include "NonCopyable.h"
#include <string>
#include <vector>
#include <iostream>
#include "imgui/imgui_impl_sdl2.h"
#include "vulkan/vulkan_core.h"

// @brief Contains the window that will display the application
class Window : public NonCopyable {
public:
	// @brief Creates an SDL_Window object and initializes SDL
	// @param dimensions - dimensions of window extent
	// @param name - Name for the created window
	Window(glm::ivec2 dimensions, const std::string &name);

	// @brief Destroys the SDL_Window
	~Window();

    // @brief Gets the window size after being resized
    void updateSize();

    // @brief Get the required Vulkan extensions that the window system requires
    // @param extensions - Populated with the required extensions
    static void getRequiredInstanceExtensions(std::vector<const char*>& extensions);

    // @brief Create the Vulkan surface that will communicate with the SDL window. Called in Device constructor
    // @param instance - Vulkan instance to associate the surface with
    VkSurfaceKHR createSurface(VkInstance instance);

    // @brief Signals that the window should close
	void closeWindow() { _windowShouldClose = true; }

    // @brief Set whether the rendering loop should be paused or not
    inline void setPauseRendering(bool value) { _pauseRendering = value; }

    // @brief Set whether the window should go fullscreen or not
	inline void setFullscreen(bool value) { _isFullscreen = value; }

    inline VkExtent2D extent() { return _windowExtent; }
    inline float aspectRatio() { return _aspectRatio; }
    inline struct SDL_Window* SDL_window() { return _window; }
    inline VkSurfaceKHR surface() { return _surface; }
    inline bool shouldClose() { return _windowShouldClose; }
    inline bool pauseRendering() { return _pauseRendering; }
    inline bool isFullscreen() { return _isFullscreen; }

private:
	struct SDL_Window* _window;
	VkExtent2D _windowExtent;
	std::string _name;

	// @brief the Vulkan surface associated with the window
	VkSurfaceKHR _surface;

	// @brief VkInstance only needed to create a surface.
	VkInstance _instance;

    float _aspectRatio;

	bool _windowShouldClose;
	bool _pauseRendering;
	bool _isFullscreen;
};
