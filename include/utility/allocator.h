#pragma once
#include "vma/vk_mem_alloc.h"
#include "utility/logger.h"
#include "renderer/device.h"
#include "renderer/instance.h"
#include "NonCopyable.h"

class DeviceMemoryManager : public NonCopyable {
public:
	DeviceMemoryManager(Device& device, Instance& instance);
	~DeviceMemoryManager();

	inline VmaAllocator allocator() const { return _vmaAllocator; }

private:
	// @brief The actual VMA allocator instance
	VmaAllocator _vmaAllocator;

	Device& _device;
	Instance& _instance;
};
