#pragma once
#include <vulkan/vulkan.h>
#include "utility/logger.h"
#include "renderer/instance.h"
#include "NonCopyable.h"
#include <iostream>

class Instance;

class DebugMessenger : public NonCopyable {
public:
	DebugMessenger(Instance& instance);
	~DebugMessenger();

	// @brief Get handle of the debug messenger
	inline VkDebugUtilsMessengerEXT& handle() { return debugMessenger; }

	// @brief Callback function that the debug messenger will use to report validation layer info
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	// @brief Fills debug messenger create info for ease of creation later
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:
	// @brief Reference to the associated Instance object
	Instance& instance;

	// @brief The Vulkan debug messenger
	VkDebugUtilsMessengerEXT debugMessenger;
};
