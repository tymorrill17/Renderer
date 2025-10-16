#pragma once
#include "renderer/shader.h"
#include "renderer/command.h"
#include "utility/allocator.h"
#include "utility/debug_messenger.h"
#include "swapchain.h"
#include "image.h"
#include "descriptor.h"
#include "pipeline.h"
#include "render_systems/render_system.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <vector>
#include <string>

struct RendererCreateInfo {
    const std::string& application_name;
    uint32_t window_width;
    uint32_t window_height;
    const std::vector<const char*>* validation_layers;
    const std::vector<const char*>* device_extensions;
};

class Renderer {
public:
    void initialize(RendererCreateInfo* renderer_info);
    void cleanup();
    void wait_for_idle();

    void draw();
    void resize_callback();
    Renderer& add_render_system(RenderSystem* render_system);

    Window window;
    Instance instance;
    DebugMessenger debug_messenger;
    Device device;
    DeviceMemoryManager device_memory_manager;
    Swapchain swapchain;
    PipelineBuilder pipeline_builder;
    std::vector<FrameSync> frame_sync;
    DrawImage draw_image;
    CommandPool command_pool;
    std::vector<Command> frame_command;
    DescriptorLayoutBuilder descriptor_layout_builder;
    DescriptorWriter descriptor_writer;
    ShaderManager shader_manager;
    std::vector<RenderSystem*> render_systems;

    uint32_t frames_in_flight;
    uint32_t frame_number;
    uint32_t frame_index;
};
