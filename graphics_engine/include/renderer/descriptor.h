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

class Renderer;
class Buffer;

struct PoolSizeRatio {
    VkDescriptorType type;
    float ratio;
};

class DescriptorAllocator {
public:
    void initialize(Device* device, uint32_t initial_sets, std::span<PoolSizeRatio> pool_size_ratios, VkDescriptorPoolCreateFlags flags = 0);
    void reset_all_descriptor_sets();
	void cleanup();

    VkDescriptorPool get_open_pool();
    VkDescriptorSet allocate_descriptor_set(VkDescriptorSetLayout layout);

    std::vector<PoolSizeRatio> pool_size_ratios;
	std::vector<VkDescriptorPool> full_pools;
	std::vector<VkDescriptorPool> open_pools;
	uint32_t sets_per_pool;
    VkDescriptorPoolCreateFlags pool_flags;

    Device* device;

private:
    VkDescriptorPool new_pool(uint32_t set_count, std::span<PoolSizeRatio> pool_size_ratios);
};

class DescriptorLayoutBuilder {
public:
    void initialize(Device* device);

    DescriptorLayoutBuilder& add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage);
    DescriptorLayoutBuilder& clear();
    VkDescriptorSetLayout build();

    Device* device;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

class DescriptorWriter {
public:
    void initialize(Device* device);

    DescriptorWriter& add_image(uint32_t binding, ImageType* image, VkSampler sampler, VkDescriptorType descriptor_type);
    DescriptorWriter& add_buffer(uint32_t binding, Buffer* buffer, VkDescriptorType descriptor_type, size_t offset = 0, size_t size = VK_WHOLE_SIZE);
    DescriptorWriter& clear();
    DescriptorWriter& write(VkDescriptorSet descriptor_set);

	Device* device;
	std::deque<VkDescriptorImageInfo>  image_infos;
	std::deque<VkDescriptorBufferInfo> buffer_infos;
	std::vector<VkWriteDescriptorSet>  set_writes;
};

class DescriptorSet {
public:
    void cleanup();

    Renderer* renderer;
    VkDescriptorSet handle;
    VkDescriptorSetLayout layout;
};

class DescriptorBuilder {
public:
    void initialize(Renderer* renderer, uint32_t max_sets, std::span<PoolSizeRatio> pool_size_ratios);
    void cleanup();

    DescriptorBuilder& add_buffer(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage, Buffer* buffer, size_t offset = 0, size_t size = VK_WHOLE_SIZE);
    DescriptorBuilder& add_image(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage, ImageType* image, VkSampler sampler);
    DescriptorBuilder& clear();
    DescriptorSet build();

    Renderer* renderer;
    DescriptorAllocator descriptor_allocator;
    DescriptorWriter descriptor_writer;
    DescriptorLayoutBuilder descriptor_layout_builder;;
};

