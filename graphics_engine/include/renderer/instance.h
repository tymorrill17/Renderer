#pragma once
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "utility/debug_messenger.h"
#include "vulkan/vulkan_core.h"
#include <vector>
#include <string>

class Instance {
public:
    void initialize(
        const std::string& app_name,
        const std::string& engine_name,
        const std::vector<const char*>* requested_validation_layers,
        const std::vector<const char*>* requested_device_extensions
    );
    void cleanup();

    static bool check_validation_layer_support(const std::vector<const char*>* requested_validation_layers);
    static void get_required_instance_extensions(std::vector<const char*>* extensions, bool validation_layers);

    VkInstance handle;
};
