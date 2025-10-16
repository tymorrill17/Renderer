#pragma once
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "device.h"
#include "image.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
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

class DescriptorPool {
public:
    void initialize(Device* device, uint32_t max_sets, std::span<PoolSizeRatio> pool_size_ratios);
    void cleanup();

    VkDescriptorSet allocate_descriptor_set(VkDescriptorSetLayout layout);
    void reset_all_descriptor_sets();

    Device* device;
    VkDescriptorPool handle;
};

class DescriptorLayoutBuilder {
public:
    void initialize(Device* device);
    void cleanup();

    DescriptorLayoutBuilder& add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage);
    DescriptorLayoutBuilder& clear();
    VkDescriptorSetLayout build();
    void destroy_all_layouts();

    Device* device;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::deque<VkDescriptorSetLayout> layout_deletion_queue;
};

class DescriptorWriter {
public:
    void initialize(Device* device);
    void cleanup();

    DescriptorWriter& add_image(uint32_t binding, ImageType* image, VkSampler sampler, VkDescriptorType descriptor_type);
//    DescriptorWriter& add_buffer(uint32_t binding, Buffer* buffer, VkDescriptorType descriptor_type, size_t offset = 0, size_t size = VK_WHOLE_SIZE);
    DescriptorWriter& clear();
    DescriptorWriter& write_descriptor_set(VkDescriptorSet descriptor_set);

	Device* device;
	std::deque<VkDescriptorImageInfo>  image_infos;
	std::deque<VkDescriptorBufferInfo> buffer_infos;
	std::vector<VkWriteDescriptorSet>  set_writes;
};

