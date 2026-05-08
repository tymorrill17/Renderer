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

    // When this semaphore is signaled, it means we have an image from the swapchain and
    // are ready to start drawing to it.
	Semaphore sem_acquired_image;

    // The fence is a CPU-side synchronization object. It signals the GPU that the CPU is done
    // issuing rendering commands
	Fence render_fence;
};
