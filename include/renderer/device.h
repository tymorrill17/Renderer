#pragma once
#include <vulkan/vulkan.h>
#include "device.h"
#include "instance.h"
#include "utility/window.h"
#include "queue_family.h"
#include "vulkan/vulkan_core.h"
#include <vector>
#include <string>
#include <set>

class Device : public NonCopyable {
public:
	Device(Instance& instance, Window& window, const std::vector<const char*>& extensions);
	~Device();

	inline VkPhysicalDevice physicalDevice() { return _physDevice; }
	inline VkPhysicalDeviceProperties physicalDeviceProperies() { return _physDeviceProperties; }
	inline VkDevice handle() { return _logicalDevice; }
	inline QueueFamilyIndices queueFamilyIndices() { return _indices; }
	inline VkQueue graphicsQueue() { return _graphQueue; }
	inline VkQueue presentQueue() { return _presQueue; }

private:
    Instance& _instance;
    Window& _window;
	VkPhysicalDevice _physDevice; // Representation of the physical GPU
    VkPhysicalDeviceProperties _physDeviceProperties; // Properties of the chosen GPU
	VkDevice _logicalDevice; // Logical representation of the physical device that the code can interact with

	QueueFamilyIndices _indices;
	VkQueue _graphQueue; // Graphics queue
	VkQueue _presQueue; // Present queue

    VkSurfaceKHR _windowSurface; // Keep track of window surface for deletion

	// @brief Verify that the selected physical device supports the requested extensions
	// @param physicalDevice - The selected physical device to check
	// @param extensions - The requested device extensions
	// @return True if the physical device supports all of extensions. False otherwise
	static bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*>& extensions);

	// @brief Queries available physical devices and selects one based on whether or not it supports required device extensions
	// @param instance - The current active instance of Vulkan
	// @param surface - The surface which the swapchain will present to
	// @param requiredExtensions - The device extensions that are required to present images to the screen
	// @return The selected VkPhysicalDevice object
	static VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions);

	// @brief Checks whether the selected physical device has swapchain support
	// @param physicalDevice - The selected physical device to check
	// @param surface - The surface which the swapchain will present to
	// @return True if the physical device has swapchain support. False otherwise
	static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};
