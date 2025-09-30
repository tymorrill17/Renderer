#pragma once
#include "NonCopyable.h"
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "device.h"
#include "buffer.h"
#include "image.h"
#include "vulkan/vulkan_core.h"
#include <span>
#include <unordered_map>
#include <vector>
#include <string>
#include <deque>

// @brief Describes how many of each type of descriptor set to make room for in the descriptor pool.
//		  Used in the initialization of the descriptor pool in the DescriptorAllocator.
struct PoolSizeRatio {
	VkDescriptorType type;
	float ratio;
};

class DescriptorPool : public NonCopyable {
public:
	DescriptorPool(Device& device, uint32_t maxSets, std::span<PoolSizeRatio> poolSizeRatios);
	~DescriptorPool();

	// @brief Allocates a descriptor set from the given layout
	// @param layout - Descriptor set layout to create the descriptor set with
    // @return The resulting descriptor set
	VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout layout);

	// @brief Clears the currently allocated descriptor sets
	void clearDescriptorSets();

	inline VkDescriptorPool pool() { return _descriptorPool; }

private:
	Device& _device;
	VkDescriptorPool _descriptorPool;
};

class DescriptorLayoutBuilder : public NonCopyable {
public:
	DescriptorLayoutBuilder(Device& device);
    ~DescriptorLayoutBuilder() { flushLayouts(); }

	// @brief Adds a binding and descriptor type to the descriptor layout builder
	// @param binding - Which binding position to assign this to
	// @param descriptorType - Which type of descriptor set to bind
	// @param shaderStages - Which shader will use this descriptor set
	// @return The handle of the builder for chaining
	DescriptorLayoutBuilder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);

	// @brief Clears the builder of current bindings
	DescriptorLayoutBuilder& clear();

	// @brief Builds a descriptor set layout with the current bindings
	VkDescriptorSetLayout build();

    // @brief Call destroy on all the descriptor set layouts created
    void flushLayouts();

private:
	Device& _device;
	std::vector<VkDescriptorSetLayoutBinding> _bindings;

    // Store the created descriptor set layouts so that they can be destroyed at program shutdown
    std::vector<VkDescriptorSetLayout> _layoutDeletionQueue;
};

// DescriptorWriter is for binding and writing the data to the GPU
class DescriptorWriter : public NonCopyable {
public:
	DescriptorWriter(Device& device);

	// @brief adds a VkDescriptorImageInfo to the imageInfos queue to be written using updateSet()
	DescriptorWriter& addImage(uint32_t binding, AllocatedImage& image, VkSampler sampler, VkDescriptorType descriptorType);

	// @brief adds a VkDescriptorBufferInfo to the bufferInfos queue to be written using updateSet()
	DescriptorWriter& addBuffer(uint32_t binding, Buffer& buffer, VkDescriptorType descriptorType, size_t offset = 0, size_t bufferSize = VK_WHOLE_SIZE);

	// @brief clears the imageInfos, bufferInfos, and writes
	DescriptorWriter& clear();

	// @brief updates and writes the set using the infos in _imageInfos and _bufferInfos
	DescriptorWriter& writeDescriptorSet(VkDescriptorSet descriptor);

private:
	Device& _device;
	std::deque<VkDescriptorImageInfo> _imageInfos;
	std::deque<VkDescriptorBufferInfo> _bufferInfos;
	std::vector<VkWriteDescriptorSet> _writes;

};



