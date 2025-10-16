#pragma once
#include <vulkan/vulkan.h>
#include "renderer/device.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>

class QueueFamilyIndices;

namespace Logger {
    // @brief Log error to stderr
    void logError(const std::string& errorMessage);
    void log(const std::string& message);

	// @brief Print a list of strings
	void printList(const std::vector<const char*>* list);

	// @brief Print a list of validation layers
	void printLayers(const char* layerCategory, const std::vector<VkLayerProperties>* layers);
	void printLayers(const char* layerCategory, const std::vector<const char*>* layers);

	// @brief Print a list of extensions
	void printExtensions(const char* extensionCategory, const std::vector<VkExtensionProperties>* extensions);
	void printExtensions(const char* extensionCategory, const std::vector<const char*>* extensions);

	// @brief Print the list of physical devices
	void printDevices(const std::vector<VkPhysicalDevice>* devices);

	// @brief Print the Vulkan version number
	void reportVersion(uint32_t version);

	// @brief Print details about the QueueFamilyIndices
	void print_queue_families(const QueueFamilyIndices& indices);

	// @brief Print details about the physical device properties
    void print_device_properties(VkPhysicalDeviceProperties physDevice);
}

