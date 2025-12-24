#include "renderer/descriptor.h"
#include "renderer/renderer.h"
#include "renderer/image.h"
#include "renderer/buffer.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"

// ---------------------------------------------- DESCRIPTOR POOL -----------------------------------------------------------------

void DescriptorPool::initialize(Device* device, uint32_t max_sets, std::span<PoolSizeRatio> pool_size_ratios) {

    this->device = device;

	std::vector<VkDescriptorPoolSize> pool_sizes;
	for (PoolSizeRatio ratio : pool_size_ratios) {
		pool_sizes.emplace_back(ratio.type, static_cast<uint32_t>(ratio.ratio * max_sets));
	}
	VkDescriptorPoolCreateInfo descriptor_pool_create_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = max_sets,
		.poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
		.pPoolSizes = pool_sizes.data()
	};
	if (vkCreateDescriptorPool(device->logical_device, &descriptor_pool_create_info, nullptr, &handle) != VK_SUCCESS) {
        Logger::logError("Failed to create descriptor pool!");
	}
}

void DescriptorPool::cleanup() {
    reset_all_descriptor_sets();
	vkDestroyDescriptorPool(device->logical_device, handle, nullptr);
}

void DescriptorPool::reset_all_descriptor_sets() {
	vkResetDescriptorPool(device->logical_device, handle, 0);
}

VkDescriptorSet DescriptorPool::allocate_descriptor_set(VkDescriptorSetLayout layout) {
	VkDescriptorSetAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = handle,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout
	};
	VkDescriptorSet set;
	if (vkAllocateDescriptorSets(device->logical_device, &alloc_info, &set) != VK_SUCCESS) {
        Logger::logError("Failed to allocate descriptor sets!");
	}
	return set;
}

// ---------------------------------------------- DESCRIPTOR LAYOUT BUILDER -----------------------------------------------------------------

void DescriptorLayoutBuilder::initialize(Device* device) {
    this->device = device;
}

DescriptorLayoutBuilder& DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage) {
	VkDescriptorSetLayoutBinding new_binding{
		.binding = binding,
		.descriptorType = descriptor_type,
		.descriptorCount = 1,
		.stageFlags = shader_stage
	};
	bindings.push_back(new_binding);
	return *this;
}

DescriptorLayoutBuilder& DescriptorLayoutBuilder::clear() {
	bindings.clear();
	return *this;
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build() {
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

    VkDescriptorSetLayout layout;

	if (vkCreateDescriptorSetLayout(device->logical_device, &descriptor_set_layout_create_info, nullptr, &layout) != VK_SUCCESS) {
        Logger::logError("Failed to build descriptor set layout!");
	}

	return layout;
}

// ---------------------------------------------- DESCRIPTOR WRITER -----------------------------------------------------------------

void DescriptorWriter::initialize(Device* device) {
    this->device = device;
}

DescriptorWriter& DescriptorWriter::add_image(uint32_t binding, ImageType* image, VkSampler sampler, VkDescriptorType descriptor_type) {
	VkDescriptorImageInfo& image_info = image_infos.emplace_back( VkDescriptorImageInfo {
		.sampler = sampler,
		.imageView = image->view,
		.imageLayout = image->layout
		});

	VkWriteDescriptorSet set_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = descriptor_type,
		.pImageInfo = &image_info
	};

	set_writes.push_back(set_write);
	return *this;
}

DescriptorWriter& DescriptorWriter::add_buffer(uint32_t binding, Buffer* buffer, VkDescriptorType descriptor_type, size_t offset, size_t size) {
 	VkDescriptorBufferInfo& buffer_info = buffer_infos.emplace_back(VkDescriptorBufferInfo {
 		.buffer = buffer->handle,
 		.offset = offset,
 		.range = size
 		});

 	VkWriteDescriptorSet set_write = {
 		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
 		.dstSet = VK_NULL_HANDLE,
 		.dstBinding = binding,
 		.descriptorCount = 1,
 		.descriptorType = descriptor_type,
 		.pBufferInfo = &buffer_info
 	};

 	set_writes.push_back(set_write);
 	return *this;
 }

DescriptorWriter& DescriptorWriter::write(VkDescriptorSet descriptor_set) {
	for (VkWriteDescriptorSet& set_write : set_writes) {
		set_write.dstSet = descriptor_set;
	}
	vkUpdateDescriptorSets(device->logical_device, static_cast<uint32_t>(set_writes.size()), set_writes.data(), 0, nullptr);
	return *this;
}

DescriptorWriter& DescriptorWriter::clear() {
	image_infos.clear();
	buffer_infos.clear();
	set_writes.clear();
	return *this;
}

// ---------------------------------------------- DESCRIPTOR SET -----------------------------------------------------------------

void DescriptorSet::cleanup() {
    vkDestroyDescriptorSetLayout(renderer->device.logical_device, layout, nullptr);
}

// ---------------------------------------------- DESCRIPTOR BUILDER -----------------------------------------------------------------

void DescriptorBuilder::initialize(Renderer* renderer, uint32_t max_sets, std::span<PoolSizeRatio> pool_size_ratios) {
    this->renderer = renderer;
    this->descriptor_pool.initialize(&renderer->device, max_sets, pool_size_ratios);
    this->descriptor_layout_builder.initialize(&renderer->device);
    this->descriptor_writer.initialize(&renderer->device);
}

void DescriptorBuilder::cleanup() {
    descriptor_pool.cleanup();
}

DescriptorBuilder& DescriptorBuilder::clear() {
    descriptor_layout_builder.clear();
    descriptor_writer.clear();
    return *this;
}

DescriptorBuilder& DescriptorBuilder::add_buffer(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage, Buffer* buffer, size_t offset, size_t size) {
    descriptor_layout_builder.add_binding(binding, descriptor_type, shader_stage);
    descriptor_writer.add_buffer(binding, buffer, descriptor_type);
    return *this;
}

DescriptorBuilder& DescriptorBuilder::add_image(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stage, ImageType* image, VkSampler sampler) {
    descriptor_layout_builder.add_binding(binding, descriptor_type, shader_stage);
    descriptor_writer.add_image(binding, image, sampler, descriptor_type);
    return *this;
}

DescriptorSet DescriptorBuilder::build() {
    DescriptorSet set;
    set.renderer = this->renderer;
    set.layout = descriptor_layout_builder.build();
    set.handle = descriptor_pool.allocate_descriptor_set(set.layout);
    descriptor_writer.write(set.handle);

    return set;
}
