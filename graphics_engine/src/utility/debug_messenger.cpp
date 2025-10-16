#include "utility/debug_messenger.h"

void DebugMessenger::initialize(Instance* instance) {

    this->instance = instance;

    // Then set up the debug messenger
    VkDebugUtilsMessengerCreateInfoEXT debug_info{};
    populate_debug_messenger_create_info(debug_info);

    // Get the pointer to the extension function
    auto create_function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->handle, "vkCreateDebugUtilsMessengerEXT");

    if (!create_function) {
        Logger::logError("Debug messenger extension not available!");
    }

    // Actually create the debug messenger
    if (create_function(instance->handle, &debug_info, nullptr, &handle) != VK_SUCCESS) {
        Logger::logError("Failed to create debug messenger!");
    }

    Logger::log("Created debug messenger.");
}

void DebugMessenger::cleanup() {
    // Get the pointer to the extension function
    auto destroy_function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->handle, "vkDestroyDebugUtilsMessengerEXT");
    destroy_function(instance->handle, handle, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void DebugMessenger::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
	create_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback
	};
}
