#include "render_systems/gui_render_system.h"
#include "render_systems/render_system.h"
#include "utility/logger.h"

std::vector<PoolSizeRatio> poolSizes = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
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

GuiRenderSystem::GuiRenderSystem(Renderer& renderer, Window& window) :
	RenderSystem(renderer),
	_window(window),
	_descriptorPool(_renderer.device(), 1000, poolSizes) {

	// Initialize core structures of ImGui
	ImGui::CreateContext();

	// Initializes ImGui for SDL
	ImGui_ImplSDL2_InitForVulkan(_window.SDL_window());

	// Initializes ImGui for Vulkan
	VkFormat swapchainImageFormat = _renderer.swapchain().imageFormat();
	VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &swapchainImageFormat
	};
	ImGui_ImplVulkan_InitInfo vulkanInit{
		.Instance = _renderer.instance().handle(),
		.PhysicalDevice = _renderer.device().physicalDevice(),
		.Device = _renderer.device().handle(),
		.Queue = _renderer.device().graphicsQueue(),
		.DescriptorPool = _descriptorPool.pool(),
		.MinImageCount = _renderer.swapchain().framesInFlight(),
		.ImageCount = _renderer.swapchain().framesInFlight(),
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.UseDynamicRendering = true,
		.PipelineRenderingCreateInfo = pipelineRenderingInfo
	};
	ImGui_ImplVulkan_Init(&vulkanInit);

	ImGui_ImplVulkan_CreateFontsTexture();
}

void GuiRenderSystem::render(Command& command) {
	static Gui& gui = Gui::getGui();
	gui.constructWindows();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command.buffer());
}

void GuiRenderSystem::getNewFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void GuiRenderSystem::endFrame() {
	ImGui::EndFrame();
}

GuiRenderSystem::~GuiRenderSystem() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}
