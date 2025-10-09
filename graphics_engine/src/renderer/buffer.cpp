#include "renderer/buffer.h"
#include "renderer/image.h"
#include "utility/allocator.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"

Buffer::Buffer(DeviceMemoryManager* allocator) :
    _deviceMemoryManager(allocator),
    _buffer(VK_NULL_HANDLE),
    _mappedData(nullptr),
    _bufferSize(0),
    _instanceCount(0),
    _instanceSize(0),
    _alignmentSize(0)
{}

Buffer::Buffer(DeviceMemoryManager* allocator, size_t instanceSize,
	uint32_t instanceCount, VkBufferUsageFlags usageFlags,
	VmaMemoryUsage memoryUsage, size_t minOffsetAlignment) :
	_deviceMemoryManager(allocator),
	_buffer(VK_NULL_HANDLE),
	_mappedData(nullptr) {

    create(instanceSize, instanceCount, usageFlags, memoryUsage);
}

Buffer::~Buffer() {
    destroy();
}

void Buffer::create(size_t instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
	VmaMemoryUsage memoryUsage, size_t minOffsetAlignment) {

    _instanceSize = instanceSize;
    _instanceCount = instanceCount;
    _bufferSize = _instanceSize * _instanceCount;
    _alignmentSize = findAlignmentSize(_instanceSize, minOffsetAlignment);

	VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = _bufferSize,
		.usage = usageFlags
	};

	VmaAllocationCreateInfo allocationCreateInfo{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memoryUsage
	};

	if (vmaCreateBuffer(_deviceMemoryManager->allocator(), &bufferCreateInfo, &allocationCreateInfo, &_buffer, &_allocation, &_allocationInfo) != VK_SUCCESS) {
        Logger::logError("Failed to create allocated buffer!");
	}
}

Buffer::Buffer(Buffer&& other) noexcept :
    _deviceMemoryManager(std::move(other._deviceMemoryManager)),
    _buffer(std::move(other._buffer)),
    _allocation(std::move(other._allocation)),
    _allocationInfo(std::move(other._allocationInfo)),
    _mappedData(std::move(other._mappedData)),
    _bufferSize(std::move(other._bufferSize)),
    _instanceCount(std::move(other._instanceCount)),
    _instanceSize(std::move(other._instanceSize)),
    _alignmentSize(std::move(other._alignmentSize)) {

    other._deviceMemoryManager = nullptr;
    other._buffer = VK_NULL_HANDLE;
    other._allocation = nullptr;
    other._mappedData = nullptr;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        _deviceMemoryManager = std::move(other._deviceMemoryManager);
        _buffer = std::move(other._buffer);
        _allocation = std::move(other._allocation);
        _allocationInfo = std::move(other._allocationInfo);
        _mappedData = std::move(other._mappedData);
        _bufferSize = std::move(other._bufferSize);
        _instanceCount = std::move(other._instanceCount);
        _instanceSize = std::move(other._instanceSize);
        _alignmentSize = std::move(other._alignmentSize);

        other._deviceMemoryManager = nullptr;
        other._buffer = VK_NULL_HANDLE;
        other._allocation = nullptr;
        other._mappedData = nullptr;
    }
    return *this;
}


void Buffer::destroy() {
    if (_mappedData)
        unmap();

    vmaDestroyBuffer(_deviceMemoryManager->allocator(), _buffer, _allocation);
}

void Buffer::map() {
	if (vmaMapMemory(_deviceMemoryManager->allocator(), _allocation, &_mappedData) != VK_SUCCESS) {
        Logger::logError("Failed to map memory to the buffer!");
	}
}

void Buffer::unmap() {
	if (_mappedData) {
		vmaUnmapMemory(_deviceMemoryManager->allocator(), _allocation);
		_mappedData = nullptr;
	}
}

void Buffer::writeData(void* data, size_t size, size_t offset) {
	if (!_mappedData) {
        Logger::logError("Trying to write to an unmapped buffer!");
	}

    // If we are writing to all of _mappedData, a simple memcpy is sufficient
	if (size == VK_WHOLE_SIZE) {
		memcpy(_mappedData, data, _bufferSize);
	} else { // otherwise, use the offset and size to find a subsection of mapped data and then a memcpy
		char* memOffset = (char*)_mappedData;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}
}

void Buffer::writeDataAtIndex(void* data, int index) {
	writeData(data, _instanceSize, index * _alignmentSize);
}

size_t Buffer::findAlignmentSize(size_t instanceSize, size_t minOffsetAlignment) {
	if (minOffsetAlignment > 0) {
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	return instanceSize;
}
