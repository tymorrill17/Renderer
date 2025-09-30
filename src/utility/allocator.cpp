#include "utility/logger.h"
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"
#include "utility/allocator.h"

DeviceMemoryManager::DeviceMemoryManager(Device& device, Instance& instance) : _device(device), _instance(instance) {
	VmaAllocatorCreateInfo allocatorCreateInfo{
		.physicalDevice = _device.physicalDevice(),
		.device = _device.handle(),
		.instance = _instance.handle()
	};
	if (vmaCreateAllocator(&allocatorCreateInfo, &_vmaAllocator) != VK_SUCCESS) {
        Logger::logError("Failed to create the VMA allocator!");
	}
}

DeviceMemoryManager::~DeviceMemoryManager() {
	vmaDestroyAllocator(_vmaAllocator);
}
