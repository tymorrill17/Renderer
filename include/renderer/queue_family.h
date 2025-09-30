#pragma once
#include <vulkan/vulkan.h>
#include "utility/logger.h"
#include <optional>
#include <vector>
#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily; // Draw command support
	std::optional<uint32_t> presentFamily; // Drawing to surface support
	inline bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	std::vector<VkQueueFamilyProperties> queueFamilyProperties; // Properties of the chosen GPU's queue families

    // @brief Find the indices of queue families with support for graphics and present commands. They may be the same queue.
    // @param physicalDevice - Physical device to query for queue families
    // @param surface - Surface object to queue present support for
    // @return The QueueFamilyIndices struct which contains indices for the graphics and present queue families. These may both be the same number
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};

