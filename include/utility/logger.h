#pragma once
#include <vulkan/vulkan.h>
#include "renderer/queue_family.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>

namespace Logger {
    // @brief Log error to stderr
    void logError(std::string errorMessage);

    void log(std::string message);

	// @brief Print a list of strings
	void printList(std::vector<const char*>& list);

	// @brief Print a list of validation layers
	void printLayers(const char* layerCategory, std::vector<VkLayerProperties>& layers);
	void printLayers(const char* layerCategory, std::vector<const char*>& layers);

	// @brief Print a list of extensions
	void printExtensions(const char* extensionCategory, std::vector<VkExtensionProperties>& extensions);
	void printExtensions(const char* extensionCategory, std::vector<const char*>& extensions);

	// @brief Print the list of physical devices
	void printDevices(std::vector<VkPhysicalDevice>& devices);

	// @brief Print the Vulkan version number
	void reportVersion(uint32_t version);

	// @brief Print details about the QueueFamilyIndices
	void log(struct QueueFamilyIndices& indices);

	// @brief Print details about the physical device properties
    void log(struct VkPhysicalDeviceProperties& physDevice);
}

