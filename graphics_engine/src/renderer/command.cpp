#include "renderer/command.h"
#include "renderer/sync.h"
#include "vulkan/vulkan_core.h"

// CommandPool --------------------------------------------------------------------------------------------------

void CommandPool::initialize(Device* device, VkCommandPoolCreateFlags flags) {

    this->device = device;

	VkCommandPoolCreateInfo command_pool_create_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = flags,
		.queueFamilyIndex = device->queue_indices.graphics_family.value()
	};

    if (vkCreateCommandPool(device->logical_device, &command_pool_create_info, nullptr, &handle) != VK_SUCCESS) {
        Logger::logError("Failed to create command pool!");
	}
}

void CommandPool::cleanup() {
	vkDestroyCommandPool(device->logical_device, handle, nullptr);
}

void CommandPool::reset(VkCommandPoolResetFlags flags) {
    vkResetCommandPool(device->logical_device, handle, flags);
}

// Command --------------------------------------------------------------------------------------------------

void Command::initialize(Device* device, CommandPool* command_pool) {

    this->device = device;
    this->command_pool = command_pool;
    in_progress = false;

	VkCommandBufferAllocateInfo allocate_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = command_pool->handle,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	if (vkAllocateCommandBuffers(device->logical_device, &allocate_info, &buffer) != VK_SUCCESS) {
        Logger::logError("Failed to allocate command buffer!");
	}
}

VkCommandBufferBeginInfo Command::command_buffer_begin_info(VkCommandBufferUsageFlags flags) {
	VkCommandBufferBeginInfo begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = flags,
		.pInheritanceInfo = nullptr
	};
	return begin_info;
}

void Command::begin() {
	if (in_progress) {
        Logger::logError("Command buffer already begun!");
	}
	VkCommandBufferBeginInfo begin_info = command_buffer_begin_info();
	if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS) {
        Logger::logError("Failed to begin command buffer!");
	}
	in_progress = true;
}

void Command::end() {
	if (!in_progress) {
        Logger::logError("Can't end a command buffer that has not begun!");
	}
	if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        Logger::logError("Failed to end command buffer!");
	}
	in_progress = false;
}

void Command::reset(VkCommandBufferResetFlags flags) {
	if (vkResetCommandBuffer(buffer, flags) != VK_SUCCESS) {
        Logger::logError("Failed to reset the command buffer!");
	}
}

void Command::submit_to_queue(VkQueue queue, FrameSync* sync) {
	VkCommandBufferSubmitInfo command_submit_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = buffer,
		.deviceMask = 0
	};
	// This semaphore waits until the previous frame has been presented
	VkSemaphoreSubmitInfo wait_semaphore_info{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = sync->present_semaphore.handle,
		.value = 1,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		.deviceIndex = 0
	};
	// This semaphore waits until the frame is fully rendered
	VkSemaphoreSubmitInfo signal_semaphore_info{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = sync->render_semaphore.handle,
		.value = 1,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		.deviceIndex = 0
	};

	VkSubmitInfo2 submit_info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = nullptr,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &wait_semaphore_info,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &command_submit_info,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signal_semaphore_info
	};

	if (vkQueueSubmit2(queue, 1, &submit_info, sync->render_fence.handle) != VK_SUCCESS) {
        Logger::logError("Failed to submit commands to queue!");
	}
}

// ImmediateCommand --------------------------------------------------------------------------------------------------

void ImmediateCommand::initialize(Device* device, CommandPool* command_pool) {
    Command::initialize(device, command_pool);
    submit_fence.initialize(device, VK_FENCE_CREATE_SIGNALED_BIT);
}

void ImmediateCommand::run_command(std::function<void(VkCommandBuffer cmd)>&& function) {
	vkResetFences(device->logical_device, 1, &submit_fence.handle);
	reset(); // Reset the command buffer

	begin();
	function(buffer);
	end();

	VkCommandBufferSubmitInfo command_submit_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = buffer,
		.deviceMask = 0
	};
	// This semaphore waits until the previous frame has been presented

	VkSubmitInfo2 submit_info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = nullptr,
		.waitSemaphoreInfoCount = 0,
		.pWaitSemaphoreInfos = nullptr,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &command_submit_info,
		.signalSemaphoreInfoCount = 0,
		.pSignalSemaphoreInfos = nullptr
	};

	if (vkQueueSubmit2(device->graphics_queue, 1, &submit_info, submit_fence.handle) != VK_SUCCESS) {
        Logger::logError("Failed to submit commands to queue!");
	}
	vkWaitForFences(device->logical_device, 1, &submit_fence.handle, true, 9999999999);
}

void ImmediateCommand::cleanup() {
    submit_fence.cleanup();
}

