#include "utility/debug_messenger.h"

DebugMessenger::DebugMessenger(Instance& instance) : instance(instance), debugMessenger(VK_NULL_HANDLE) {
	if (instance.validationLayersEnabled()) {

		// Then set up the debug messenger
		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		populateDebugMessengerCreateInfo(debugInfo);

		// Get the pointer to the extension function
		auto createFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance.handle(), "vkCreateDebugUtilsMessengerEXT");

		if (!createFunc) {
            Logger::logError("Debug messenger extension not available!");
		}

		// Actually create the debug messenger
		if (createFunc(instance.handle(), &debugInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            Logger::logError("Failed to create debug messenger!");
		}

        std::cout << "Created debug messenger." << std::endl;
	}
}

DebugMessenger::~DebugMessenger() {
	if (instance.validationLayersEnabled()) {
		// Get the pointer to the extension function
		auto destroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance.handle(), "vkDestroyDebugUtilsMessengerEXT");

		destroyFunc(instance.handle(), debugMessenger, nullptr);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void DebugMessenger::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback
	};
}
