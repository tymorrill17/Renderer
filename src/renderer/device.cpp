#include "renderer/device.h"
#include "renderer/queue_family.h"
#include "renderer/swapchain.h"
#include "vulkan/vulkan_core.h"
#include <iostream>

static VkPhysicalDeviceFeatures deviceFeatures{};

static VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
													 .synchronization2 = true,
													 .dynamicRendering = true };

static VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
													 .descriptorIndexing = true,
													 .bufferDeviceAddress = true };

Device::Device(Instance& instance, Window& window, const std::vector<const char*>& extensions) :
    _instance(instance),
    _window(window),
	_physDevice(VK_NULL_HANDLE),
	_logicalDevice(VK_NULL_HANDLE),
	_graphQueue(VK_NULL_HANDLE),
	_presQueue(VK_NULL_HANDLE),
    _windowSurface(VK_NULL_HANDLE) {

	// Create the surface for the passed-in window. I don't necessarily like it being here, but we are keeping window creation separate from the engine
    // and the surface needs an instance to be created
	_windowSurface = _window.createSurface(_instance.handle());

	// Select the physical device to be used for rendering
	_physDevice = selectPhysicalDevice(_instance.handle(), _window.surface(), extensions);

	// Query the physical device properties
	vkGetPhysicalDeviceProperties(_physDevice, &_physDeviceProperties);
    Logger::log(_physDeviceProperties);

	// Find the queue families and assign their indices
	_indices = QueueFamilyIndices::findQueueFamilies(_physDevice, _window.surface());
	Logger::log(_indices);
	std::set<uint32_t> uniqueQueueFamilies = { _indices.graphicsFamily.value(), _indices.presentFamily.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	// Populate queue create infos
	float priority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = queueFamily,
		.queueCount = 1,
		.pQueuePriorities = &priority};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Chain the desired features together using pNext before feeding them into deviceCreateInfo
	VkPhysicalDeviceFeatures2 versionFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.features = deviceFeatures };
	features12.pNext = &features13;
	versionFeatures.pNext = &features12;

	// Set up the logical device
	VkDeviceCreateInfo deviceCreateInfo{
	.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	.pNext = &versionFeatures,
	.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
	.pQueueCreateInfos = queueCreateInfos.data(),
	.enabledLayerCount = instance.validationLayersEnabled() ? static_cast<uint32_t>(Instance::requestedValidationLayers.size()) : 0,
	.ppEnabledLayerNames = instance.validationLayersEnabled() ? Instance::requestedValidationLayers.data() : VK_NULL_HANDLE,
	.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
	.ppEnabledExtensionNames = extensions.data()
	};

	if (vkCreateDevice(_physDevice, &deviceCreateInfo, nullptr, &_logicalDevice) != VK_SUCCESS) {
        Logger::logError("Failed to create logical device!");
	}
    std::cout << "Vulkan device successfully created." << std::endl;

	// Get handles for the graphics and present queues
	vkGetDeviceQueue(_logicalDevice, _indices.graphicsFamily.value(), 0, &_graphQueue);
	vkGetDeviceQueue(_logicalDevice, _indices.presentFamily.value(), 0, &_presQueue);
}

Device::~Device() {
	if (_windowSurface) {
		vkDestroySurfaceKHR(_instance.handle(), _windowSurface, nullptr);
	}
	vkDestroyDevice(_logicalDevice, nullptr);
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*>& extensions) {

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	if (extensionCount < 1)
		return false;

	// Get supported extensions
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
	Logger::printExtensions("Available Device Extensions:", availableExtensions);
	Logger::printExtensions("Required Device Extensions:", extensions);

	std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	if (requiredExtensions.empty()) {
        std::cout << "Required extensions are supported by the physical device!" << std::endl;
		return true;
	}
    std::cout << "Required extensions are NOT supported, selecting next device..." << std::endl;
	return false;
}

bool Device::isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

	QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(physicalDevice, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, Instance::requestedDeviceExtensions);

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	bool discreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	return indices.isComplete() && extensionsSupported && discreteGPU;
}

VkPhysicalDevice Device::selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions) {

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		Logger::logError("Failed to find any GPU with Vulkan support!");

    std::cout << "Selecting physical device..." << std::endl;

	// Get available devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	Logger::printDevices(devices);

	VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, surface)) {
			selectedDevice = device;
            std::cout << "Selected a suitable physical device: " << std::endl;
			break;
		}
	}

	// Fall back on default GPU if no other suitable ones
	if (selectedDevice == VK_NULL_HANDLE) {
        std::cout << "Failed to find a suitable physical device, selecting default: " << std::endl;
		selectedDevice = devices[0];
	}

	return selectedDevice;
}
