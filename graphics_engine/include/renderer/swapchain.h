#pragma once
#include <vulkan/vulkan.h>

#include "utility/logger.h"
#include "renderer/device.h"
#include "renderer/sync.h"
#include "renderer/image.h"
#include "vulkan/vulkan_core.h"

#include <vector>
#include <iostream>

class Window;

// @brief Stores supported swapchain attributes
struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

class Swapchain {
public:
    void initialize(Device* device, Window* window);
    void create_swapchain();
    void cleanup();

    void check_for_window_resize(VkResult result);
    void recreate();
    void acquire_next_image(FrameSync* sync);
    void present_to_screen(VkQueue queue, FrameSync* sync);
    SwapchainImage& current_image() { return images[image_index]; }

    static SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    static VkExtent2D find_swapchain_extent(VkSurfaceCapabilitiesKHR surface_capabilities, Window* window);

    static VkSurfaceFormatKHR select_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    static VkPresentModeKHR select_swapchain_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);

    Device* device;
    Window* window;

    VkSwapchainKHR handle;
    std::vector<SwapchainImage> images;
    uint32_t image_index;
    bool window_resized;

    VkFormat image_format;
    VkExtent2D extent;
    uint32_t frames_in_flight;

};
