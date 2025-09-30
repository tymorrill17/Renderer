#include "renderer/renderer.h"
#include "renderer/frame.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>

Renderer::Renderer(Window& window) :
	_instance("EngineTest", "VulkanEngineV2", true),
    _window(window),
	_debugMessenger(_instance),
	_device(_instance, _window, Instance::requestedDeviceExtensions),
	_deviceMemoryManager(_device, _instance),
	_swapchain(_device, _window),
	_pipelineBuilder(_device),
    // _frames(_swapchain.framesInFlight(), Frame(_device)),
	_drawImage(&_device, &_deviceMemoryManager, VkExtent3D{ _window.extent().width, _window.extent().height, 1 }, _swapchain.imageFormat(),
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY, VkMemoryAllocateFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), VK_IMAGE_ASPECT_COLOR_BIT),
    _commandPool(&_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
	_descriptorLayoutBuilder(_device),
	_descriptorWriter(_device),
    _frameNumber(0) {

	_frames.reserve(_swapchain.framesInFlight());
    _perFrameCmd.reserve(_swapchain.framesInFlight());
	for (int i = 0; i < _frames.capacity(); i++) {
		_frames.emplace_back(&_device);
        _perFrameCmd.emplace_back(&_device, &_commandPool);
	}

    std::cout << "Engine Initiated!" << std::endl;
}

static VkRenderingInfoKHR renderingInfoKHR(VkExtent2D extent, uint32_t colorAttachmentCount, VkRenderingAttachmentInfo* pColorAttachmentInfos, VkRenderingAttachmentInfo* pDepthAttachmentInfo) {
	VkRenderingInfoKHR renderInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.pNext = nullptr,
		.layerCount = 1,
		.colorAttachmentCount = colorAttachmentCount,
		.pColorAttachments = pColorAttachmentInfos,
		.pDepthAttachment = pDepthAttachmentInfo
	};
	renderInfo.renderArea.extent = extent;
	renderInfo.renderArea.offset = { 0, 0 };
	return renderInfo;
}

uint32_t Renderer::getFrameIndex() {
    return _frameNumber % _swapchain.framesInFlight();
}

Frame& Renderer::getCurrentFrame() {
	return _frames[getFrameIndex()];
}

Frame& Renderer::getFrame(int index) {
	return _frames[index];
}

Renderer& Renderer::addRenderSystem(RenderSystem* renderSystem) {
	_renderSystems.push_back(renderSystem);
	return *this;
}

void Renderer::renderAllSystems() {
	// First, wait for the the last frame to render
	VkFence currentRenderFence = getCurrentFrame().renderFence().handle();
	vkWaitForFences(_device.handle(), 1, &currentRenderFence, true, 1000000000);
	// Here would be the place to delete all objects from the previous frame (like descriptor sets, etc)
	vkResetFences(_device.handle(), 1, &currentRenderFence);

	// Next, request current frame's image from the swapchain
	_swapchain.acquireNextImage(&getCurrentFrame().presentSemaphore(), nullptr);

	// Get the current frame's command buffer
	Command* cmd = &_perFrameCmd[getFrameIndex()];
	cmd->reset(); // Reset before adding more commands to be safe
	cmd->begin(); // Begin the command buffer

	// Transition the draw image to a writable format
	_drawImage.transitionImage(*cmd, VK_IMAGE_LAYOUT_GENERAL);

	// Now the rendering info struct needs to be filled with the leftover info that the renderpass usually handles
	VkClearValue clearColorValue{ .color{ 0.0f, 0.0f, 0.0f, 1.0f } };
	VkRenderingAttachmentInfoKHR colorAttachmentInfo = Image::attachmentInfo(_drawImage.imageView(), &clearColorValue, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingInfoKHR renderingInfo = renderingInfoKHR(_window.extent(), 1, &colorAttachmentInfo, nullptr);

	// Transition draw image to a color attachment
	_drawImage.transitionImage(*cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	// Set dynamic viewport and scissor
	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(_window.extent().width),
		.height = static_cast<float>(_window.extent().height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = _window.extent()
	};

	vkCmdBeginRendering(cmd->buffer(), &renderingInfo);

	// First, set the dynamic states: viewport and scissor
	vkCmdSetViewport(cmd->buffer(), 0, 1, &viewport);
	vkCmdSetScissor(cmd->buffer(), 0, 1, &scissor);

	// We want to do imgui rendering here I think. It's either first or last. Maybe last since it's an overlay

	// Call render() for each RenderSystem. Note that the order in which these systems are called matters.
	for (auto* renderSystem : _renderSystems) {
		renderSystem->render(*cmd);
	}

	vkCmdEndRendering(cmd->buffer());

	// Transition images for copying and then presenting
	// Draw image is going to be copied to the swapchain image, so transition it to a transfer source layout
	_drawImage.transitionImage(*cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	// Swapchain image needs to be transitioned to a transfer destination layout
	_swapchain.image(_swapchain.imageIndex()).transitionImage(*cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Image::copyImageOnGPU(*cmd, &_drawImage, &_swapchain.image(_swapchain.imageIndex()));

	// Transition swapchain image to a presentation-ready layout
	_swapchain.image(_swapchain.imageIndex()).transitionImage(*cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	cmd->end();
	cmd->submitToQueue(_device.graphicsQueue(), getCurrentFrame()); // Submit the command buffer
	_swapchain.presentToScreen(_device.presentQueue(), getCurrentFrame(), _swapchain.imageIndex()); // Present to screen

	_frameNumber++;
}

void Renderer::resizeCallback() {
	if (_swapchain.resizeRequested()) {
        _window.updateSize();
		_swapchain.recreate();
		_drawImage.recreate({ _window.extent().width, _window.extent().height, 1 });
	}
}

void Renderer::waitForIdle() {
	vkDeviceWaitIdle(_device.handle());
}

void Renderer::shutdown() {
    waitForIdle();
//    _perFrameCmd.clear();
    //_commandPool.reset();
}





