#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "device.h"
#include "command.h"
#include "sync.h"

class Command;

class Frame : public NonCopyable {
public:
	Frame(Device* device);

	inline Semaphore& presentSemaphore() { return _presentSemaphore; }
	inline Semaphore& renderSemaphore() { return _renderSemaphore; }
	inline Fence& renderFence() { return _renderFence; }

    Frame(Frame&& other) noexcept;
    Frame& operator=(Frame&& other) noexcept;

private:
	Semaphore _presentSemaphore;
	Semaphore _renderSemaphore;
	Fence _renderFence;
};
