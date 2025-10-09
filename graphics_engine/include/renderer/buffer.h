#pragma once
#include "NonCopyable.h"
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "utility/allocator.h"


class Buffer : public NonCopyable {
public:
    Buffer(DeviceMemoryManager* deviceMemoryManager);
	Buffer(DeviceMemoryManager* deviceMemoryManager, size_t instanceSize,
		uint32_t instanceCount, VkBufferUsageFlags usageFlags,
		VmaMemoryUsage memoryUsage, size_t minOffsetAlignment = 1);
    ~Buffer();

    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    // @brief Create the buffer object.
    void create(size_t instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
		VmaMemoryUsage memoryUsage, size_t minOffsetAlignment = 1);

    // @brief Destroys the buffer object
    void destroy();

	// @brief Maps CPU-accessible pointer to the buffer on the GPU
	void map();

	// @brief Unmap the CPU-accessible pointer
	void unmap();

	// @brief Writes data to the buffer. The data written is either the entire capacity, or a specified size and offset
	// @param data - The data to be written to the buffer
	// @param size - (optional) The size of the data to be written
	// @param offset - (optional) Amount to offset the writing in the buffer
	void writeData(void* data, size_t size = VK_WHOLE_SIZE, size_t offset = 0);

	// @brief Writes an instance of the buffer's data at the index
	// @param data - The data to be written to the buffer
	// @param index - Which instance to write to
	void writeDataAtIndex(void* data, int index);

	inline VkBuffer buffer() { return _buffer; }
	inline VmaAllocation allocation() { return _allocation; }
	inline VmaAllocationInfo allocationInfo() { return _allocationInfo; }
	inline size_t bufferSize() { return _bufferSize; }
	inline uint32_t instanceCount() { return _instanceCount; }
	inline size_t instanceSize() { return _instanceSize; }
	inline size_t alignmentSize() { return _alignmentSize; }

private:
	DeviceMemoryManager* _deviceMemoryManager;

	VkBuffer _buffer; // Vulkan buffer object
	VmaAllocation _allocation; // vma allocation object
	VmaAllocationInfo _allocationInfo; // Info used to allocate the buffer with vma

	void* _mappedData;

	size_t _bufferSize; // The total size in bytes of the buffer
	uint32_t _instanceCount; // How many instances of the struct being stored by the buffer (usually the number of frames in flight)
	size_t _instanceSize; // The size in bytes of a single instance of the struct being stored by the buffer
	size_t _alignmentSize; // The device-specific alignment size

	static size_t findAlignmentSize(size_t instanceSize, size_t minOffsetAlignment);
};
