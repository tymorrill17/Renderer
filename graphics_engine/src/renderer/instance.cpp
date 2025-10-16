#include "renderer/instance.h"
#include "utility/window.h"
#include "utility/logger.h"
#include <iostream>

void Instance::initialize(
        const std::string& app_name,
        const std::string& engine_name,
        const std::vector<const char*>* requested_validation_layers,
        const std::vector<const char*>* requested_device_extensions) {

    bool validation_layers_enabled = requested_validation_layers->size() > 0;

	if (validation_layers_enabled && !check_validation_layer_support(requested_validation_layers)) {
        Logger::logError("Validation layers requested, but are not supported!");
	}

	// Find version of Vulkan
	uint32_t instance_version;
	vkEnumerateInstanceVersion(&instance_version);
    Logger::reportVersion(instance_version);

	// User-specified info about application
	VkApplicationInfo app_info{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = app_name.c_str(),
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.pEngineName = engine_name.c_str(),
		.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.apiVersion = instance_version
	};

	// Populate the instance create info
	VkInstanceCreateInfo instance_create_info{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info
	};

	// Request instance extensions
	std::vector<const char*> extensions;
	get_required_instance_extensions(&extensions, validation_layers_enabled);
	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_create_info.ppEnabledExtensionNames = extensions.data();

	// Request validation layers if enabled
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
	if (validation_layers_enabled) {
		// Request validation layers
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(requested_validation_layers->size());
		instance_create_info.ppEnabledLayerNames = requested_validation_layers->data();

		DebugMessenger::populate_debug_messenger_create_info(debug_create_info);
		instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debug_create_info;
	}
	else {
		instance_create_info.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&instance_create_info, nullptr, &handle) != VK_SUCCESS) {
        Logger::logError("Failed to create a Vulkan instance!");
	}

    Logger::log("Created Vulkan instance.");
}

void Instance::cleanup() {
	vkDestroyInstance(handle, nullptr);
}

bool Instance::check_validation_layer_support(const std::vector<const char*>* requested_validation_layers) {
	// Query the instance for supported validation layers
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> supported_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, supported_layers.data());

    Logger::printLayers("Layers Supported by Instance:", &supported_layers);

	// Loop through the requested validation layers and confirm they are all supported
	for (const char* layer : *requested_validation_layers) {
		bool found_layer = false;
		for (const auto& layer_properties : supported_layers) {
			if (strcmp(layer, layer_properties.layerName) == 0) {
				found_layer = true;
				break;
			}
		}
		if (!found_layer) {
            Logger::logError("Layer \"" + std::string(layer) + "\" is not supported!");
			return false;
		}
	}
    Logger::log("All requested layers are supported!");
	return true;
}

void Instance::get_required_instance_extensions(std::vector<const char*>* extensions, bool validation_layers_requested) {

	Window::get_required_instance_extensions(extensions);

	if (validation_layers_requested) {
		extensions->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

    Logger::printExtensions("Required Instance Extensions:", extensions);
}
