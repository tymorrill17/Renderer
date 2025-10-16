#pragma once
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "device.h"
#include "vulkan/vulkan_core.h"

class Semaphore {
public:
    void initialize(Device* device, VkSemaphoreCreateFlags flags = 0U);
    void cleanup();

    Device* device;
    VkSemaphore handle;
};

class Fence {
public:
    void initialize(Device* device, VkFenceCreateFlags flags = 0U);
    void cleanup();

    Device* device;
    VkFence handle;
};

class FrameSync {
public:
    void initialize(Device* device);
    void cleanup();

	Semaphore present_semaphore;
	Semaphore render_semaphore;
	Fence render_fence;
};
