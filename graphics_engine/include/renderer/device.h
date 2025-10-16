#pragma once
#include <vulkan/vulkan.h>
#include "renderer/device.h"
#include "renderer/instance.h"
#include "utility/window.h"
#include "vulkan/vulkan_core.h"
#include <vector>
#include <string>
#include <optional>
#include <set>

class QueueFamilyIndices {
public:
    std::optional<uint32_t> graphics_family; // Draw command support
	std::optional<uint32_t> present_family; // Drawing to surface support
    std::vector<VkQueueFamilyProperties> queue_family_properties; // Properties of the chosen GPU's queue families

    inline bool complete() { return graphics_family.has_value() && present_family.has_value(); }
    static QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
};

class Window;
class Instance;

class Device {
public:
    void initialize(
        Instance* instance,
        Window* window,
        const std::vector<const char*>* requested_validation_layers,
        const std::vector<const char*>* requested_extensions
    );
    void cleanup();

    static bool check_device_extension_support(VkPhysicalDevice physical_device, const std::vector<const char*>* extensions);
    static VkPhysicalDevice select_physical_device(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>* device_extensions);
    static bool device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const std::vector<const char*>* device_extensions);

    Instance* instance;
    Window* window;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;

    QueueFamilyIndices queue_indices;
    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSurfaceKHR window_surface;

};
