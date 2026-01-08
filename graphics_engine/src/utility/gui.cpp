#include "utility/gui.h"
#include "renderer/image.h"
#include "renderer/renderer.h"
#include <iostream>

std::vector<PoolSizeRatio> pool_sizes = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
	{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
	{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
	{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
};

void Gui::initialize(Renderer* renderer) {
    this->renderer = renderer;
	descriptor_allocator.initialize(&renderer->device, 1000, pool_sizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

	// Initialize core structures of ImGui
	ImGui::CreateContext();

	// Initializes ImGui for SDL
	ImGui_ImplSDL3_InitForVulkan(renderer->window.sdl_window);

	// Initializes ImGui for Vulkan
	VkPipelineRenderingCreateInfoKHR pipeline_rendering_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &renderer->swapchain.image_format,
        .depthAttachmentFormat = renderer->depth_image.format,
	};

	ImGui_ImplVulkan_InitInfo vulkan_init{
		.Instance = renderer->instance.handle,
		.PhysicalDevice = renderer->device.physical_device,
		.Device = renderer->device.logical_device,
        .QueueFamily = renderer->device.queue_indices.graphics_family.value(),
		.Queue = renderer->device.graphics_queue,
		.DescriptorPool = descriptor_allocator.get_open_pool(),
		.MinImageCount = renderer->frames_in_flight,
		.ImageCount = renderer->frames_in_flight,
		.UseDynamicRendering = true,
	};

    vulkan_init.PipelineInfoMain.PipelineRenderingCreateInfo = pipeline_rendering_info;

	ImGui_ImplVulkan_Init(&vulkan_init);
}

void Gui::cleanup() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
    descriptor_allocator.cleanup();
}

void Gui::process_inputs(SDL_Event* event) {
	ImGui_ImplSDL3_ProcessEvent(event);
}

void Gui::add_widget(const std::string& window_name, const std::function<void()>& widget) {
	widget_dictionary[window_name].push_back(widget);
}

void Gui::construct_windows() {
    if (widget_dictionary.empty()) return;

	for (auto& [window_name, widgets] : widget_dictionary) {
		ImGui::Begin(window_name.c_str());
		for (auto& widget : widgets) {
			widget();
		}
		ImGui::End();
	}
	widget_dictionary.clear();
}

void Gui::start_frame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void Gui::end_frame() {
	ImGui::EndFrame();
}

void Gui::draw(Command* cmd) {
    construct_windows();

    Image::transition_image(cmd, &renderer->swapchain.current_image(), VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingAttachmentInfoKHR color_attachment_info = Image::color_attachment_info(renderer->swapchain.current_image().view, nullptr, VK_IMAGE_LAYOUT_GENERAL);
    VkExtent2D draw_extent{
        renderer->swapchain.current_image().extent.width,
        renderer->swapchain.current_image().extent.height
    };
	VkRenderingInfoKHR render_info = Renderer::rendering_info(draw_extent, 1, &color_attachment_info, nullptr);
	vkCmdBeginRendering(cmd->buffer, &render_info);

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->buffer);

	vkCmdEndRendering(cmd->buffer);
}
