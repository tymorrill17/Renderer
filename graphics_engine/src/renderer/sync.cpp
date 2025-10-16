#include "renderer/sync.h"

// SEMAPHORE --------------------------------------------------------------------------------------------------------------------------

void Semaphore::initialize(Device* device, VkSemaphoreCreateFlags flags) {

    this->device = device;

	VkSemaphoreCreateInfo semaphore_info{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = flags
	};
	if (vkCreateSemaphore(device->logical_device, &semaphore_info, nullptr, &handle) != VK_SUCCESS) {
        Logger::logError("Failed to create semaphore!");
	}
}

void Semaphore::cleanup() {
	vkDestroySemaphore(device->logical_device, handle, nullptr);
}

// FENCE --------------------------------------------------------------------------------------------------------------------------

void Fence::initialize(Device* device, VkFenceCreateFlags flags) {

    this->device = device;

	VkFenceCreateInfo fence_info{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = flags,
	};
	if (vkCreateFence(device->logical_device, &fence_info, nullptr, &handle) != VK_SUCCESS) {
        Logger::logError("Failed to create fence!");
	}
}

void Fence::cleanup() {
	vkDestroyFence(device->logical_device, handle, nullptr);
}

// FRAMESYNC ----------------------------------------------------------------------------------------------------------------------

void FrameSync::initialize(Device* device) {
    present_semaphore.initialize(device);
    render_semaphore.initialize(device);
	render_fence.initialize(device, VK_FENCE_CREATE_SIGNALED_BIT);
}

void FrameSync::cleanup() {
    present_semaphore.cleanup();
    render_semaphore.cleanup();
    render_fence.cleanup();
}

