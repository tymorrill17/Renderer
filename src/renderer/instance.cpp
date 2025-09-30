#include "renderer/instance.h"
#include <iostream>

std::vector<const char*> Instance::requestedValidationLayers = {
	"VK_LAYER_KHRONOS_validation" // Standard validation layer preset
};
std::vector<const char*> Instance::requestedDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME // Necessary extension to use swapchains
};

Instance::Instance(const char* appName, const char* engineName, bool enableValidationLayers) :
	instance(VK_NULL_HANDLE),
    enableValidationLayers(enableValidationLayers) {

	if (enableValidationLayers && !checkValidationLayerSupport()) {
        Logger::logError("Validation layers requested, but are not supported!");
	}

	// Find version of Vulkan
	uint32_t instanceVersion;
	vkEnumerateInstanceVersion(&instanceVersion);
    Logger::reportVersion(instanceVersion);

	// User-specified info about application
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = appName,
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.pEngineName = engineName,
		.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.apiVersion = instanceVersion
	};

	// Populate the instance create info
	VkInstanceCreateInfo instanceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo
	};

	// Request instance extensions
	std::vector<const char*> extensions;
	getRequiredInstanceExtensions(extensions, enableValidationLayers);
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	// Request validation layers if enabled
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		// Request validation layers
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(Instance::requestedValidationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = Instance::requestedValidationLayers.data();

		DebugMessenger::populateDebugMessengerCreateInfo(debugCreateInfo);
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugCreateInfo;
	}
	else {
		instanceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
        Logger::logError("Failed to create a Vulkan instance!");
	}

    std::cout << "Created Vulkan instance." << std::endl;
}

Instance::~Instance() {
	vkDestroyInstance(instance, nullptr);
}

bool Instance::checkValidationLayerSupport() {

	// Query the instance for supported validation layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> supportedLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

    Logger::printLayers("Layers Supported by Instance:", supportedLayers);

	// Loop through the requested validation layers and confirm they are all supported
	for (const char* layer : Instance::requestedValidationLayers) {
		bool foundLayer = false;
		for (const auto& layerProperties : supportedLayers) {
			if (strcmp(layer, layerProperties.layerName) == 0) {
				foundLayer = true;
				break;
			}
		}
		if (!foundLayer) {
			std::cout << "Layer \"" << layer << "\" is not supported!" << std::endl;
			return false;
		}
	}
    std::cout << "All requested layers are supported!" << std::endl;
	return true;
}

void Instance::getRequiredInstanceExtensions(std::vector<const char*>& extensions, bool requestedValidationLayers) {

	Window::getRequiredInstanceExtensions(extensions);

	if (requestedValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

    Logger::printExtensions("Required Instance Extensions:", extensions);
}
