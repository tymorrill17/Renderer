#pragma once
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "utility/allocator.h"


class Buffer {
public:
    void cleanup();

	void map();
	void unmap();

	void write_data(void* data, size_t size = VK_WHOLE_SIZE, size_t offset = 0);
	void write_data_at_index(void* data, int index);

	DeviceMemoryManager* device_memory_manager;
	VkBuffer handle;
	VmaAllocation allocation;

	void* mapped_data;

	size_t total_bytes; // The total size in bytes of the buffer
	size_t instance_count; // How many instances of the struct being stored by the buffer (usually the number of frames in flight)
	size_t instance_bytes; // The size in bytes of a single instance of the struct being stored by the buffer
	size_t alignment; // The device-specific alignment size

	static size_t find_alignment_size(size_t instance_bytes, size_t minimum_offset_alignment);
};
