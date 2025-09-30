#pragma once
#include "NonCopyable.h"
#include "renderer/shader.h"
#include "utility/logger.h"
#include "renderer/command.h"
#include "utility/allocator.h"
#include "utility/debug_messenger.h"
#include "swapchain.h"
#include "image.h"
#include "descriptor.h"
#include "pipeline.h"
#include "render_systems/render_system.h"
#include "utility/logger.h"
#include <cstdint>

class Swapchain;
class AllocatedImage;

class Renderer : public NonCopyable {
public:
	// @brief Construct and initialize the Vulkan
    // @param window - reference to a window object that will be rendered to and will provide input data
	Renderer(Window& window);

	// @brief Renders each RenderSystem to the frame and presents it.
    // Each type of thing that will be rendered will be part of some render system
	void renderAllSystems();

	// @brief Handles changes that need to be made when the window is resized
	void resizeCallback();

	// @brief Adds renderSystem to the end of the renderSystems list
    // @param renderSystem - pointer to a render system to add
	// @return Returns the Renderer handle in order to chain together adds
	Renderer& addRenderSystem(RenderSystem* renderSystem);

    // @brief Gets the swapchain index of the current frame
    uint32_t getFrameIndex();

	// @brief Gets the frame object of the current frame by finding frameNumber % swapchain.framesInFlight
    // @return Frame object at current frame
	Frame& getCurrentFrame();

    // @brief Gets the frame object at the given index
    // @param frame index (must be < swapchain.framesInFlight)
    // @return Frame object at index
	Frame& getFrame(int index);

    // @brief Waits for the device to be idle
	void waitForIdle();

    // @brief Make sure all GPU processes are finished. This must be called before the program ends.
    void shutdown();

	inline Device& device() { return _device; }
	inline Swapchain& swapchain() { return _swapchain; }
	inline Instance& instance() { return _instance; }
	inline PipelineBuilder& pipelineBuilder() { return _pipelineBuilder; }
	inline DescriptorLayoutBuilder& descriptorLayoutBuilder() { return _descriptorLayoutBuilder; }
	inline DescriptorWriter& descriptorWriter() { return _descriptorWriter; }
	inline DeviceMemoryManager& deviceMemoryManager() { return _deviceMemoryManager; }
	inline ShaderManager& shaderManager() { return _shaderManager; }

private:
	Window& _window; // Main window to render to. It is a reference because the renderer does not create it.

    // Everything else is created by the renderer and lives in the Renderer object
	Instance _instance; // @brief Vulkan instance object
    DebugMessenger _debugMessenger; // Vulkan debug messenger callback for validation layers
	Device _device; // Vulkan device object containing physical and logical devices
	DeviceMemoryManager _deviceMemoryManager; // Wrapper over VMA that handles buffer allocation and freeing
	Swapchain _swapchain; // The swapchain handles presents draw images to the window
	PipelineBuilder _pipelineBuilder; // Pipeline builder handles graphics and compute pipeline creation since that is tied to the renderer

    // Frame data and draw image
	std::vector<Frame> _frames; // Contains command buffers and sync objects for each frame in the swapchain
	AllocatedImage _drawImage; // Image that gets rendered to then copied to the swapchain image(s)
    CommandPool _commandPool;
    std::vector<Command> _perFrameCmd;

    // Descriptor sets
	DescriptorLayoutBuilder _descriptorLayoutBuilder; // Build descriptor set layouts
	DescriptorWriter _descriptorWriter; // Binds and writes descriptor layouts

    // Shaders
    ShaderManager _shaderManager;

    // Render systems dictate the nature of how objects that use them are rendered
    std::vector<RenderSystem*> _renderSystems; // List of render systems that get called each frame

    // Renderer statistics
    uint32_t _frameNumber; // Keeps track of the number of rendered frames
};
