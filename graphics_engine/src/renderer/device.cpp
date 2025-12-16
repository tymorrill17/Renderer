#include "renderer/device.h"
#include "SDL3/SDL_error.h"
#include "renderer/swapchain.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"
#include "SDL3/SDL_vulkan.h"
#include <iostream>

static VkPhysicalDeviceFeatures device_features{};
static VkPhysicalDeviceVulkan13Features features_13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
													 .synchronization2 = true,
													 .dynamicRendering = true };
static VkPhysicalDeviceVulkan12Features features_12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
													 .descriptorIndexing = true,
													 .bufferDeviceAddress = true };
static VkPhysicalDeviceVulkan11Features features_11{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
													 .shaderDrawParameters = true };

QueueFamilyIndices QueueFamilyIndices::find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	// Get queue families from physical device
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

	indices.queue_family_properties = queue_families;

	// Iterate through the families and find the ones we care about
	bool found = false;
	for (int i = 0; !found && i < queue_families.size(); i++) {
		const auto& family = queue_families[i];

		// Find graphics support
		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphics_family = i;

		// Find surface presentation support
		VkBool32 present_queue_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_queue_support);
		if (present_queue_support)
			indices.present_family = i;

		found = indices.complete();
	}
	return indices;
}

void Device::initialize(Instance* instance, Window* window, const std::vector<const char*>* requested_validation_layers, const std::vector<const char*>* requested_extensions) {

    this->instance = instance;
    this->window = window;

    // Create the surface for the passed-in window. I don't necessarily like it being here, but we are keeping window creation separate from the engine
    // and the surface needs an instance to be created
    if (!SDL_Vulkan_CreateSurface(window->sdl_window, instance->handle, nullptr, &window_surface)) {
        SDL_GetError();
        Logger::logError("Failed to create window surface!");
    }

	// Select the physical device to be used for rendering
	physical_device = select_physical_device(instance->handle, window_surface, requested_extensions);

	// Query the physical device properties
    VkPhysicalDeviceProperties physical_device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    Logger::print_device_properties(physical_device_properties);

	// Find the queue families and assign their indices
	queue_indices = QueueFamilyIndices::find_queue_families(physical_device, window_surface);
	Logger::print_queue_families(queue_indices);
	std::set<uint32_t> unique_queue_families = { queue_indices.graphics_family.value(), queue_indices.present_family.value() };
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

	// Populate queue create infos
	float priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = queue_family,
		.queueCount = 1,
		.pQueuePriorities = &priority};
		queue_create_infos.push_back(queue_create_info);
	}

	// Chain the desired features together using pNext before feeding them into deviceCreateInfo
	VkPhysicalDeviceFeatures2 version_features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.features = device_features };
	features_12.pNext = &features_13;
    features_11.pNext = &features_12;
	version_features.pNext = &features_11;

	// Set up the logical device
	VkDeviceCreateInfo device_create_info{
	.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	.pNext = &version_features,
	.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
	.pQueueCreateInfos = queue_create_infos.data(),
	.enabledLayerCount = static_cast<uint32_t>(requested_validation_layers->size()),
	.ppEnabledLayerNames = requested_validation_layers->data(),
	.enabledExtensionCount = static_cast<uint32_t>(requested_extensions->size()),
	.ppEnabledExtensionNames = requested_extensions->data()
	};

	if (vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device) != VK_SUCCESS) {
        Logger::logError("Failed to create logical device!");
	}
    Logger::log("Vulkan device successfully created.");

	// Get handles for the graphics and present queues
	vkGetDeviceQueue(logical_device, queue_indices.graphics_family.value(), 0, &graphics_queue);
	vkGetDeviceQueue(logical_device, queue_indices.present_family.value(), 0, &present_queue);
}

void Device::cleanup() {
    SDL_Vulkan_DestroySurface(instance->handle, window_surface, nullptr);
	vkDestroyDevice(logical_device, nullptr);
}

bool Device::check_device_extension_support(VkPhysicalDevice physical_device, const std::vector<const char*>* extensions) {

	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

	if (extension_count < 1)
		return false;

	// Get supported extensions
	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());
	Logger::printExtensions("Available Device Extensions:", &available_extensions);
	Logger::printExtensions("Required Device Extensions:", extensions);

	std::set<std::string> required_extensions(extensions->begin(), extensions->end());
	for (const auto& extension : available_extensions) {
		required_extensions.erase(extension.extensionName);
	}

	if (required_extensions.empty()) {
        Logger::log("Required extensions are supported by the physical device!");
		return true;
	}
    Logger::log("Required extensions are NOT supported, selecting next device...");
	return false;
}

bool Device::device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const std::vector<const char*>* device_extensions) {

	QueueFamilyIndices indices = QueueFamilyIndices::find_queue_families(physical_device, surface);

	bool extensions_supported = check_device_extension_support(physical_device, device_extensions);

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);
	bool discrete_GPU = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	return indices.complete() && extensions_supported && discrete_GPU;
}

static std::string get_physical_device_name(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    return std::string(device_properties.deviceName);
}

VkPhysicalDevice Device::select_physical_device(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>* device_extensions) {

	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

	if (device_count == 0)
		Logger::logError("Failed to find any GPU with Vulkan support!");

    Logger::log("Selecting physical device...");

	// Get available devices
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
	Logger::printDevices(&devices);

	VkPhysicalDevice selected_device = VK_NULL_HANDLE;
	for (const auto& device : devices) {
		if (device_suitable(device, surface, device_extensions)) {
			selected_device = device;
            Logger::log("Selected a suitable physical device: " + get_physical_device_name(selected_device));
			break;
		}
	}

	// Fall back on default GPU if no other suitable ones
	if (selected_device == VK_NULL_HANDLE) {
        selected_device = devices[0];
        Logger::log("Failed to find a suitable physical device, selecting default: " + get_physical_device_name(selected_device));
	}

	return selected_device;
}
