#include "renderer/renderer.h"
#include "renderer/image.h"
#include "renderer/sync.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>

static VkRenderingInfoKHR rendering_info(VkExtent2D extent, uint32_t color_attachment_count, VkRenderingAttachmentInfo* color_attachment_infos, VkRenderingAttachmentInfo* depth_attachment_info) {
	VkRenderingInfoKHR render_info{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.pNext = nullptr,
		.layerCount = 1,
		.colorAttachmentCount = color_attachment_count,
		.pColorAttachments = color_attachment_infos,
		.pDepthAttachment = depth_attachment_info
	};
	render_info.renderArea.extent = extent;
	render_info.renderArea.offset = { 0, 0 };
	return render_info;
}

void Renderer::initialize(RendererCreateInfo* renderer_info) {

    window.initialize(renderer_info->window_width, renderer_info->window_height, renderer_info->application_name);
    instance.initialize(renderer_info->application_name, "GraphicsEngine", renderer_info->validation_layers, renderer_info->device_extensions);
    debug_messenger.initialize(&instance);
    device.initialize(&instance, &window, renderer_info->validation_layers, renderer_info->device_extensions);
    device_memory_manager.initialize(&device, &instance);
    swapchain.initialize(&device, &window);
    frames_in_flight = swapchain.frames_in_flight;
    pipeline_builder.initialize(&device);

    frame_sync.reserve(frames_in_flight);
    for (int i_frame = 0; i_frame < frames_in_flight; i_frame++) {
        FrameSync sync;
        sync.initialize(&device);
        frame_sync.push_back(sync);
    }

    draw_image.initialize(
        &device,
        &device_memory_manager,
        VkExtent3D{ window.extent.width, window.extent.height, 1 },
        swapchain.image_format,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    depth_image.initialize(
        &device,
        &device_memory_manager,
        VkExtent3D{ window.extent.width, window.extent.height, 1 },
        VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    command_pool.initialize(&device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    frame_command.reserve(frames_in_flight);
    for (int i_frame = 0; i_frame < frames_in_flight; i_frame++) {
        Command command;
        command.initialize(&device, &command_pool);
        frame_command.push_back(command);
    }
    immediate_command.initialize(&device, &command_pool);

    descriptor_layout_builder.initialize(&device);
    descriptor_writer.initialize(&device);
    shader_manager.initialize();
    asset_manager.initialize(this);
    frame_number = 0;
    frame_index = 0;

    Logger::log("Renderer Initialized!");
}

void Renderer::cleanup() {
    wait_for_idle();

    descriptor_writer.cleanup();
    descriptor_layout_builder.cleanup();
    // TODO: Should there be a wait for idle before destroying command pool?
    command_pool.cleanup();
    immediate_command.cleanup();
    draw_image.cleanup();
    depth_image.cleanup();
    for (int i_frame = 0; i_frame < frames_in_flight; i_frame++) {
        frame_sync[i_frame].cleanup();
    }
    swapchain.cleanup();
    device_memory_manager.cleanup();
    device.cleanup();
    debug_messenger.cleanup();
    instance.cleanup();
    window.cleanup();
}

void Renderer::wait_for_idle() {
    vkDeviceWaitIdle(device.logical_device);
}

Renderer& Renderer::add_render_system(RenderSystem* render_system) {
	render_systems.push_back(render_system);
	return *this;
}

void Renderer::draw() {

    if (window.pause_rendering){
        return;
    }

	VkFence frame_render_fence = frame_sync[frame_index].render_fence.handle;
	vkWaitForFences(device.logical_device, 1, &frame_render_fence, true, 1000000000);
	vkResetFences(device.logical_device, 1, &frame_render_fence);

	swapchain.acquire_next_image(&frame_sync[frame_index]);

	Command* cmd = &frame_command[frame_index];
	cmd->reset();
	cmd->begin();

	// Transition the draw image to a writable format
    Image::transition_image(cmd, &draw_image, VK_IMAGE_LAYOUT_GENERAL);
    Image::transition_image(cmd, &depth_image, VK_IMAGE_LAYOUT_GENERAL);

	VkClearValue clear_value{ .color{ 0.0f, 0.0f, 0.0f, 1.0f } };
	VkRenderingAttachmentInfoKHR color_attachment_info = Image::color_attachment_info(draw_image.view, &clear_value, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingAttachmentInfoKHR depth_attachment_info = Image::depth_attachment_info(depth_image.view, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingInfoKHR render_info = rendering_info(window.extent, 1, &color_attachment_info, &depth_attachment_info);

	// Transition draw image to a color attachment
    // Image::transition_image(cmd, &draw_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(window.extent.width),
		.height = static_cast<float>(window.extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = window.extent
	};

	vkCmdBeginRendering(cmd->buffer, &render_info);

	vkCmdSetViewport(cmd->buffer, 0, 1, &viewport);
	vkCmdSetScissor(cmd->buffer, 0, 1, &scissor);

	// Call render() for each RenderSystem. Note that the order in which these systems are called matters.
	for (auto* render_system : render_systems) {
		render_system->render(cmd);
	}

	vkCmdEndRendering(cmd->buffer);

	// Transition images for copying and then presenting
	// Draw image is going to be copied to the swapchain image, so transition it to a transfer source layout
    Image::transition_image(cmd, &draw_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	// Swapchain image needs to be transitioned to a transfer destination layout
    Image::transition_image(cmd, &swapchain.images[swapchain.image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Image::copy_image_to_GPU(cmd, &draw_image, &swapchain.images[swapchain.image_index]);

	// Transition swapchain image to a presentation-ready layout
    Image::transition_image(cmd, &swapchain.images[swapchain.image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	cmd->end();
	cmd->submit_to_queue(device.graphics_queue, &frame_sync[frame_index]);
	swapchain.present_to_screen(device.present_queue, &frame_sync[frame_index]);

	frame_number++;
    frame_index = frame_number % frames_in_flight;
}

void Renderer::resize_callback() {
	if (swapchain.window_resized) {
        window.update_after_resize();
		swapchain.recreate();
		draw_image.recreate({ window.extent.width, window.extent.height, 1 });
	}
}

Buffer Renderer::create_buffer(size_t instance_bytes, size_t instance_count,
    VkBufferUsageFlags vk_memory_usage, VmaMemoryUsage vma_memory_usage,
    size_t minimum_offset_alignment) {

    Buffer new_buffer;

    new_buffer.device_memory_manager = &this->device_memory_manager;
    new_buffer.instance_bytes = instance_bytes;
    new_buffer.instance_count = instance_count;

    new_buffer.total_bytes = instance_bytes * instance_count;
    new_buffer.alignment = Buffer::find_alignment_size(instance_bytes, minimum_offset_alignment);

	VkBufferCreateInfo buffer_create_info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = new_buffer.total_bytes,
		.usage = vk_memory_usage
	};

	VmaAllocationCreateInfo allocation_create_info{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = vma_memory_usage
	};

	if (vmaCreateBuffer(new_buffer.device_memory_manager->allocator, &buffer_create_info, &allocation_create_info, &new_buffer.handle, &new_buffer.allocation, nullptr) != VK_SUCCESS) {
        Logger::logError("Failed to create allocated buffer!");
	}

    return new_buffer;
}
