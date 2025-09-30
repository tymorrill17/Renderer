#include "renderer/sync.h"

// SEMAPHORE --------------------------------------------------------------------------------------------------------------------------

Semaphore::Semaphore(Device* device, VkSemaphoreCreateFlags flags) : _device(device), _flags(flags), _semaphore(VK_NULL_HANDLE) {
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = _flags
	};
	if (vkCreateSemaphore(_device->handle(), &semaphoreInfo, nullptr, &_semaphore) != VK_SUCCESS) {
        Logger::logError("Failed to create semaphore!");
	}
}

Semaphore::~Semaphore() {
	vkDestroySemaphore(_device->handle(), _semaphore, nullptr);
}

Semaphore::Semaphore(Semaphore&& other) noexcept :
    _device(std::move(other._device)),
    _semaphore(std::move(other._semaphore)),
    _flags(std::move(other._flags)) {
    _device = nullptr;
    _semaphore = VK_NULL_HANDLE;
}

Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
    if (this != &other) {
        _device = std::move(other._device);
        _semaphore = std::move(other._semaphore);
        _flags = std::move(other._flags);
        other._device = nullptr;
        other._semaphore = VK_NULL_HANDLE;
    }
    return *this;
}

// FENCE --------------------------------------------------------------------------------------------------------------------------

Fence::Fence(Device* device, VkFenceCreateFlags flags) : _device(device), _flags(flags), _fence(VK_NULL_HANDLE) {
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = _flags,
	};
	if (vkCreateFence(_device->handle(), &fenceInfo, nullptr, &_fence) != VK_SUCCESS) {
        Logger::logError("Failed to create fence!");
	}
}

Fence::~Fence() {
	vkDestroyFence(_device->handle(), _fence, nullptr);
}

Fence::Fence(Fence&& other) noexcept :
    _device(std::move(other._device)),
    _fence(std::move(other._fence)),
    _flags(std::move(other._flags)) {
    _device = nullptr;
    _fence = VK_NULL_HANDLE;
}

Fence& Fence::operator=(Fence&& other) noexcept {
    if (this != &other) {
        _device = std::move(other._device);
        _fence = std::move(other._fence);
        _flags = std::move(other._flags);
        other._device = nullptr;
        other._fence = VK_NULL_HANDLE;
    }
    return *this;
}

