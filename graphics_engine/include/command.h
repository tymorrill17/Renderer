#pragma once
#include "vulkan/vulkan.h"
#include "logger.h"
#include "sync.h"
#include "vulkan/vulkan_core.h"
#include <functional>
#include <memory>

class Device;
class Command;

class CommandPool {
public:
    void initialize(Device* device, VkCommandPoolCreateFlags flags);
    void cleanup();

    void reset(VkCommandPoolResetFlags flags = 0U);
    Command create_command();

    Device* device;
    VkCommandPool handle;
};

class Command {
public:
    void reset(VkCommandBufferResetFlags flags = 0U);
    void begin();
    void end();
    void submit_to_queue(VkQueue queue, FrameSync* frame_sync, Semaphore* render_semaphore);

    static VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    Device* device;
    CommandPool* command_pool;
    VkCommandBuffer buffer;
    bool in_progress;
};

class ImmediateCommand {
public:
    void initialize(Device* device);
    void cleanup();

    void run_command(std::function<void(Command* immediate_command)>&& function);

    Device* device;

    Command command;
    CommandPool pool;
    Fence submit_fence;
};

