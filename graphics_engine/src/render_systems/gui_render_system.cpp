#include "render_systems/gui_render_system.h"
#include "render_systems/render_system.h"
#include "utility/logger.h"

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

void GuiRenderSystem::initialize(Renderer* renderer) {
	descriptor_pool.initialize(&renderer->device, 1000, pool_sizes);

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
		.DescriptorPool = descriptor_pool.handle,
		.MinImageCount = renderer->frames_in_flight,
		.ImageCount = renderer->frames_in_flight,
		.UseDynamicRendering = true,
	};

    vulkan_init.PipelineInfoMain.PipelineRenderingCreateInfo = pipeline_rendering_info;

	ImGui_ImplVulkan_Init(&vulkan_init);
}

void GuiRenderSystem::render(Command* command) {
	static Gui& gui = Gui::get_gui();
	gui.construct_windows();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command->buffer);
}

void GuiRenderSystem::start_frame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void GuiRenderSystem::end_frame() {
	ImGui::EndFrame();
}

void GuiRenderSystem::cleanup() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
    descriptor_pool.cleanup();
}
