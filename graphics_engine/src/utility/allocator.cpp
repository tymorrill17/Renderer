#include "utility/logger.h"
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"
#include "utility/allocator.h"

void DeviceMemoryManager::initialize(Device* device, Instance* instance) {

    this->device = device;
    this->instance = instance;

	VmaAllocatorCreateInfo allocator_create_info{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = device->physical_device,
		.device = device->logical_device,
		.instance = instance->handle,
	};

	if (vmaCreateAllocator(&allocator_create_info, &allocator) != VK_SUCCESS) {
        Logger::logError("Failed to create the VMA allocator!");
	}
}

void DeviceMemoryManager::cleanup() {
	vmaDestroyAllocator(allocator);
}
