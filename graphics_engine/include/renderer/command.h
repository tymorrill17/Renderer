#pragma once
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "renderer/sync.h"
#include "vulkan/vulkan_core.h"
#include <functional>
#include <memory>

class Device;

class CommandPool {
public:
    void initialize(Device* device, VkCommandPoolCreateFlags flags);
    void cleanup();

    void reset(VkCommandPoolResetFlags flags = 0U);

    Device* device;
    VkCommandPool handle;
};

class Command {
public:
    virtual void initialize(Device* device, CommandPool* command_pool);

    void reset(VkCommandBufferResetFlags flags = 0U);
    void begin();
    void end();
    void submit_to_queue(VkQueue queue, FrameSync* sync);

    static VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    Device* device;
    CommandPool* command_pool;
    VkCommandBuffer buffer;
    bool in_progress;
};

class ImmediateCommand : public Command {
public:
    void initialize(Device* device, CommandPool* command_pool) override;
    void cleanup();

    void run_command(std::function<void(VkCommandBuffer cmd)>&& function);

    Fence submit_fence;
};

