#include "renderer/buffer.h"
#include "renderer/image.h"
#include "utility/allocator.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"

void Buffer::cleanup() {
    if (is_mapped) unmap();

    vmaDestroyBuffer(device_memory_manager->allocator, handle, allocation);
}

void Buffer::map() {
    if (is_mapped) return;

	if (vmaMapMemory(device_memory_manager->allocator, allocation, &mapped_data) != VK_SUCCESS) {
        Logger::logError("Failed to map memory to the buffer!");
        return;
	}
    is_mapped = true;
}

void Buffer::unmap() {
	if (is_mapped) {
		vmaUnmapMemory(device_memory_manager->allocator, allocation);
		mapped_data = nullptr;
        is_mapped = false;
	}
}

void Buffer::write_data(void* data, size_t size, size_t offset) {
	if (!is_mapped) map();

    // If we are writing to all of mapped_data, a simple memcpy is sufficient
	if (size == VK_WHOLE_SIZE) {
		memcpy(mapped_data, data, total_bytes);
	} else { // otherwise, use the offset and size to find a subsection of mapped data and then a memcpy
		char* memory_offset = (char*)mapped_data; // char because it's 1 byte
		memory_offset += offset;
		memcpy(memory_offset, data, size);
	}
}

void Buffer::write_data_at_index(void* data, int index) {
	if (!is_mapped) {
        map();
	}
	write_data(data, instance_bytes, index * alignment);
}

size_t Buffer::find_alignment_size(size_t instance_bytes, size_t minimum_offset_alignment) {
	if (minimum_offset_alignment > 0) {
		return (instance_bytes + minimum_offset_alignment - 1) & ~(minimum_offset_alignment - 1);
	}
	return instance_bytes;
}
