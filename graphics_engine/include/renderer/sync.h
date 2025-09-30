#pragma once
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "device.h"
#include "NonCopyable.h"

class Semaphore : public NonCopyable {
public:
	Semaphore(Device* device, VkSemaphoreCreateFlags flags = 0U);
	~Semaphore();

    Semaphore(Semaphore&& other) noexcept;
    Semaphore& operator=(Semaphore&& other) noexcept;

	inline VkSemaphore handle() { return _semaphore; }

	void cleanup();

private:
	Device* _device;
	VkSemaphore _semaphore;
	VkSemaphoreCreateFlags _flags;
};

class Fence : public NonCopyable {
public:
	Fence(Device* device, VkFenceCreateFlags flags = 0U);
	~Fence();

    Fence(Fence&& other) noexcept;
    Fence& operator=(Fence&& other) noexcept;

	inline VkFence handle() { return _fence; }

	void cleanup();

private:
	Device* _device;
	VkFence _fence;
	VkFenceCreateFlags _flags;
};
