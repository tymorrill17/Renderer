#pragma once
#include <vulkan/vulkan.h>
#include "utility/logger.h"
#include "renderer/instance.h"
#include <iostream>

class Instance;

class DebugMessenger {
public:
    void initialize(Instance* instance);
    void cleanup();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
    );
	static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);

    Instance* instance;
	VkDebugUtilsMessengerEXT handle;
};
