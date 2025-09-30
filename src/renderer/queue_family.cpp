#include "renderer/queue_family.h"

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

	QueueFamilyIndices indices;

	// Get queue families from physical device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	indices.queueFamilyProperties = queueFamilies;

	// Iterate through the families and find the ones we care about
	bool found = false;
	for (int i = 0; !found && i < queueFamilies.size(); i++) {
		const auto& family = queueFamilies[i];

		// Find graphics support
		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		// Find surface presentation support
		VkBool32 presentQueueSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentQueueSupport);
		if (presentQueueSupport)
			indices.presentFamily = i;

		found = indices.isComplete();
	}

	return indices;
}
