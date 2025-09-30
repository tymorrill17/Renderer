#include "renderer/command.h"
#include "renderer/frame.h"
#include "vulkan/vulkan_core.h"

// CommandPool --------------------------------------------------------------------------------------------------

CommandPool::CommandPool(Device* device, VkCommandPoolCreateFlags flags) :
    _device(device),
    _commandPool(VK_NULL_HANDLE) {

	VkCommandPoolCreateInfo commandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = flags,
		.queueFamilyIndex = _device->queueFamilyIndices().graphicsFamily.value()
	};

    if (vkCreateCommandPool(_device->handle(), &commandPoolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        Logger::logError("Failed to create command pool!");
	}
}

CommandPool::~CommandPool() {
	vkDestroyCommandPool(_device->handle(), _commandPool, nullptr);
}

CommandPool::CommandPool(CommandPool&& other) noexcept :
    _device(other._device),
    _commandPool(std::move(other._commandPool)) {

    other._device = nullptr;
    other._commandPool = VK_NULL_HANDLE;
}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept {
    if (this != &other) {
        _device = std::move(other._device);
        _commandPool = std::move(other._commandPool);
        other._device = nullptr;
        other._commandPool = VK_NULL_HANDLE;
    }
    return *this;
}

void CommandPool::reset(VkCommandPoolResetFlags flags) {
    vkResetCommandPool(_device->handle(), _commandPool, flags);
}

// Command --------------------------------------------------------------------------------------------------

Command::Command(Device* device, CommandPool* commandPool) :
	_device(device),
	_commandPool(commandPool),
	_commandBuffer(VK_NULL_HANDLE),
	_inProgress(false) {

	// Now allocate the command buffer
	allocateCommandBuffer();
}

Command::Command(Command&& other) noexcept :
    _device(other._device),
    _commandPool(std::move(other._commandPool)),
    _commandBuffer(std::move(other._commandBuffer)),
    _inProgress(std::move(other._inProgress)) {

    other._commandPool = nullptr;
    other._commandBuffer = VK_NULL_HANDLE;
}

Command& Command::operator=(Command&& other) noexcept {
    if (this != &other) {
        _device = std::move(other._device);
        _commandPool = std::move(other._commandPool);
        _commandBuffer = std::move(other._commandBuffer);
        _inProgress = std::move(other._inProgress);
        other._device = nullptr;
        other._commandPool = nullptr;
        other._commandBuffer = VK_NULL_HANDLE;
    }
    return *this;
}

void Command::allocateCommandBuffer(VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = _commandPool->handle(),
		.level = level,
		.commandBufferCount = 1,
	};
	if (vkAllocateCommandBuffers(_device->handle(), &allocateInfo, &_commandBuffer) != VK_SUCCESS) {
        Logger::logError("Failed to allocate command buffer!");
	}
}

VkCommandBufferBeginInfo Command::commandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = flags,
		.pInheritanceInfo = nullptr
	};
	return beginInfo;
}

void Command::begin() {
	if (_inProgress) {
        Logger::logError("Command buffer already begun!");
	}
	VkCommandBufferBeginInfo beginInfo = commandBufferBeginInfo();
	if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS) {
        Logger::logError("Failed to begin command buffer!");
	}
	_inProgress = true;
}

void Command::end() {
	if (!_inProgress) {
        Logger::logError("Can't end a command buffer that has not begun!");
	}
	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
        Logger::logError("Failed to end command buffer!");
	}
	_inProgress = false;
}

void Command::reset(VkCommandBufferResetFlags flags) const {
	if (vkResetCommandBuffer(_commandBuffer, flags) != VK_SUCCESS) {
        Logger::logError("Failed to reset the command buffer!");
	}
}

void Command::submitToQueue(VkQueue queue, Frame& frame) {
	VkCommandBufferSubmitInfo cmdSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = _commandBuffer,
		.deviceMask = 0
	};
	// This semaphore waits until the previous frame has been presented
	VkSemaphoreSubmitInfo waitSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = frame.presentSemaphore().handle(),
		.value = 1,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		.deviceIndex = 0
	};
	// This semaphore waits until the frame is fully rendered
	VkSemaphoreSubmitInfo signalSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = frame.renderSemaphore().handle(),
		.value = 1,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		.deviceIndex = 0
	};

	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = nullptr,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &waitSemaphoreInfo,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo
	};

	if (vkQueueSubmit2(queue, 1, &submitInfo, frame.renderFence().handle()) != VK_SUCCESS) {
        Logger::logError("Failed to submit commands to queue!");
	}
}

// ImmediateCommand --------------------------------------------------------------------------------------------------

ImmediateCommand::ImmediateCommand(Device* device, CommandPool* commandPool) :
	Command(device, commandPool),
	_submitFence(device, VK_FENCE_CREATE_SIGNALED_BIT) {}

void ImmediateCommand::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
	VkFence fence = _submitFence.handle();
	vkResetFences(_device->handle(), 1, &fence);
	reset(); // Reset the command buffer

	begin();
	function(_commandBuffer);
	end();

	submitToQueue(_device->graphicsQueue());
	vkWaitForFences(_device->handle(), 1, &fence, true, 9999999999);
}

void ImmediateCommand::submitToQueue(VkQueue queue) {
	VkCommandBufferSubmitInfo cmdSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = _commandBuffer,
		.deviceMask = 0
	};
	// This semaphore waits until the previous frame has been presented

	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = nullptr,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = nullptr,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = nullptr
	};

	if (vkQueueSubmit2(queue, 1, &submitInfo, _submitFence.handle()) != VK_SUCCESS) {
        Logger::logError("Failed to submit commands to queue!");
	}
}
