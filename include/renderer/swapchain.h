#pragma once
#include <vulkan/vulkan.h>

#include "utility/logger.h"
#include "device.h"
#include "frame.h"
#include "NonCopyable.h"
#include "image.h"

#include <vector>
#include <iostream>

class Window;

// @brief Stores supported swapchain attributes
struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain : public NonCopyable {
public:
	Swapchain(Device& device, Window& window);
	~Swapchain();

	// @brief Recreate the swapchain as a result of window resizing
	void recreate();

    // @brief get the index of the next swapchain image that is ready to be presented
	void acquireNextImage(Semaphore* semaphore, Fence* fence);

    // @brief Submit to queue a request to present the frame to the surface
	void presentToScreen(VkQueue queue, Frame& frame, uint32_t imageIndex);

    inline VkSwapchainKHR handle() { return _swapchain; }
    inline VkFormat imageFormat() { return _imageFormat; }
    inline VkExtent2D extent() { return _extent; }
    inline size_t imageCount() { return _images.size(); }
    inline uint32_t imageIndex() { return _imageIndex; }
    inline SwapchainSupportDetails supportDetails() { return _supportDetails; }
    inline SwapchainImage& image(uint32_t index) { return _images[index]; }
    inline uint32_t framesInFlight() { return _framesInFlight; }
    inline bool resizeRequested() { return _resizeRequested; }

    // @brief Queries swapchain support attributes
    static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

    // @brief Sets the swapchain extent based on the current window size
    static VkExtent2D setSwapchainExtent(VkSurfaceCapabilitiesKHR capabilities, Window& window);

private:
	Device& _device;
	Window& _window;

	SwapchainSupportDetails _supportDetails;
	VkSwapchainKHR _swapchain;
	std::vector<SwapchainImage> _images;

	VkFormat _imageFormat; // @brief Format of the swapchain images
	VkExtent2D _extent; // @brief Extent of the swapchain image views
	uint32_t _framesInFlight; // @brief How many frames the swapchain contains and can be rendered in parallel
	uint32_t _imageIndex; // @brief The index of the current swapchain image being rendered to
	bool _resizeRequested; // @brief Flag triggered when the window is resized to signal the recreation of the swapchain

    // @brief Portable constructor for swapchains for use in constructor and recreate method
	void createSwapchain();

    // @brief Portable destructor for swapchains for use in destructor and recreate method
	void cleanup();

    // @brief Choose the swpachain surface format from a list of available formats
	static VkSurfaceFormatKHR selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    // @brief Choose the swapchain present mode from a list of available modes
	static VkPresentModeKHR selectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
};
