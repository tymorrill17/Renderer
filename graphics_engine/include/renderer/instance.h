#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "utility/logger.h"
#include "utility/debug_messenger.h"
#include "utility/window.h"
#include "vulkan/vulkan_core.h"
#include <vector>

class Instance : public NonCopyable {
public:
	Instance(const char* appName, const char* engineName, bool enableValidationLayers);
	~Instance();

    inline bool validationLayersEnabled() const { return enableValidationLayers; }
    inline VkInstance handle() { return instance; }

    static std::vector<const char*> requestedValidationLayers; // Requested validation layers to enable
    static std::vector<const char*> requestedDeviceExtensions; // Requested device extensions to use

private:
	VkInstance instance;
    bool enableValidationLayers; // Should validation layers be enabled.

	// @brief Verify that the instance supports the requested validation layers.
	// @return True if requested validation layers in Instance::requestedValidationLayers are supported. False if not.
	static bool checkValidationLayerSupport();

	// @brief Queries the window system for API-specific instance extensions that are needed. Also enabled validation layer extension if validation layers are enabled.
	// @param extensions - Required extension names are filled here.
	// @param validationLayers - Validation layers enabled or not?
	static void getRequiredInstanceExtensions(std::vector<const char*>& extensions, bool validationLayers);

};
