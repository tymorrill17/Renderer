#include "utility/logger.h"
#include <iostream>
#include <ostream>

void Logger::logError(const std::string& errorMessage) {
    std::cerr << errorMessage << std::endl;
    std::flush(std::cerr);
}

void Logger::log(const std::string& message) {
    std::cout << message << std::endl;
}

void Logger::printLayers(const char* layerCategory, const std::vector<VkLayerProperties>* layers) {
	std::cout << layerCategory << std::endl;
	for (const auto& layer : *layers) {
		std::cout << "\t" << layer.layerName << std::endl;
	}
}
void Logger::printLayers(const char* layerCategory, const std::vector<const char*>* layers) {
	std::cout << layerCategory << std::endl;
	printList(layers);
}

void Logger::printExtensions(const char* extensionCategory, const std::vector<VkExtensionProperties>* extensions) {
	std::cout << extensionCategory << std::endl;
	for (const auto& extension : *extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}
}

void Logger::printExtensions(const char* extensionCategory, const std::vector<const char*>* extensions) {
	std::cout << extensionCategory << std::endl;
	printList(extensions);
}

void Logger::printList(const std::vector<const char*>* list) {
	for (const auto& member : *list) {
		std::cout << "\t" << member << std::endl;
	}
}

void Logger::reportVersion(uint32_t version) {
	std::cout << "Engine using Vulkan Variant: " << VK_API_VERSION_VARIANT(version)
		<< ", Major: " << VK_API_VERSION_MAJOR(version)
		<< ", Minor: " << VK_API_VERSION_MINOR(version)
		<< ", PATCH: " << VK_API_VERSION_PATCH(version) << std::endl;
}

void Logger::print_queue_families(const QueueFamilyIndices& indices) {
	std::cout << "There are " << indices.queue_family_properties.size() << " queue families in the GPU." << std::endl;
	for (uint32_t i = 0; i < indices.queue_family_properties.size(); i++) {
		VkQueueFamilyProperties family = indices.queue_family_properties[i];
		std::cout << "Queue Family (" << i << "):" << std::endl;
		std::cout << "\tSupports " << family.queueFlags << std::endl;
		std::cout << "\tHas " << family.queueCount << " queues" << std::endl;
	}

	std::cout << "Chosen queues to utilize:" << std::endl;
	if (indices.graphics_family.has_value())
		std::cout << "\tGraphics Queue (" << indices.graphics_family.value() << ")" << std::endl;

	if (indices.present_family.has_value())
		std::cout << "\t Present Queue (" << indices.present_family.value() << ")" << std::endl;
 }

void Logger::print_device_properties(VkPhysicalDeviceProperties physDevice) {
	std::cout << "Device name: " << physDevice.deviceName << std::endl;
	std::cout << "\tDevice type: ";
	switch (physDevice.deviceType) {
	case(VK_PHYSICAL_DEVICE_TYPE_CPU):
		std::cout << "CPU";
		break;
	case(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
		std::cout << "Discrete GPU";
		break;
	case(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
		std::cout << "Integrated GPU";
		break;
	case(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
		std::cout << "Virtual GPU";
		break;
	default:
		std::cout << "Other";
	}
	std::cout << std::endl;
}

void Logger::printDevices(const std::vector<VkPhysicalDevice>* devices) {
	std::cout << "List of physical devices: " << std::endl;
	for (const auto& device : *devices) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		print_device_properties(properties);
	}
}
