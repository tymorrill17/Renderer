#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "utility/logger.h"
#include "renderer/sync.h"
#include "NonCopyable.h"
#include "vulkan/vulkan_core.h"
#include <functional>
#include <memory>

class Device;
class Frame;

class CommandPool : public NonCopyable {
public:
    CommandPool(Device* device, VkCommandPoolCreateFlags flags);
    ~CommandPool();

    CommandPool(CommandPool&&) noexcept;
    CommandPool& operator=(CommandPool&&) noexcept;

	// @brief Resets the command pool. Command buffers are not destroyed, but they are all reset to an initial state
    void reset(VkCommandPoolResetFlags flags = 0);

    inline VkCommandPool handle() { return _commandPool; }

private:
    Device* _device;
	VkCommandPool _commandPool;
};

class Command : public NonCopyable {
public:
	Command(Device* device, CommandPool* commandPool); // Or use an existing command pool

    Command(Command&& other) noexcept;
    Command& operator=(Command&& other) noexcept;

    // @brief Resets the command buffer
    void reset(VkCommandBufferResetFlags flags = 0) const;

	// @brief Begins the command buffer. Don't forget to end the command buffer too
	void begin();

	// @brief Ends the command buffer. This shouldn't be called unless the command buffer has been begun
	void end();

	// @brief Submits the current command buffer to the specified queue
	// @param queue - Queue to submit the command buffer to
	// @param frame - The current frame waiting for rendering. This object contains the sync objects needed to submit properly
	void submitToQueue(VkQueue queue, Frame& frame);

    inline CommandPool* pool() { return _commandPool; }
    inline VkCommandBuffer buffer() { return _commandBuffer; }

	// @brief Populates a command buffer begin info struct
	static VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

protected:
    Device* _device;
    CommandPool* _commandPool;
	VkCommandBuffer _commandBuffer;
	bool _inProgress;

    // @brief Allocates a command buffer from the command pool
    void allocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
};

class ImmediateCommand : public Command {
public:
	ImmediateCommand(Device* device, CommandPool* commandPool); // Or use an existing command pool

	inline Fence& fence() { return _submitFence; }

	// @brief Immediately submit a command to the graphics queue
	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

private:
	Fence _submitFence;

    // Submit the immediate command to the queue
    void submitToQueue(VkQueue queue);
};
