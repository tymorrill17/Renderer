#include "renderer/descriptor.h"
#include "renderer/renderer.h"
#include "renderer/image.h"
#include "renderer/buffer.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"

// ---------------------------------------------- DESCRIPTOR POOL -----------------------------------------------------------------

constexpr const int MAX_SETS = 4096;

void DescriptorAllocator::initialize(Device* device, uint32_t initial_sets, std::span<PoolSizeRatio> pool_size_ratios, VkDescriptorPoolCreateFlags flags) {
    this->device = device;
    this->pool_size_ratios.clear();
    for (auto ratio : pool_size_ratios) {
        this->pool_size_ratios.push_back(ratio);
    }
    this->pool_flags = flags;

    VkDescriptorPool pool = new_pool(initial_sets, pool_size_ratios);

    sets_per_pool = initial_sets * 1.5; // Next allocation, allow for more sets. Just like how vectors work
}

void DescriptorAllocator::cleanup() {
    for (auto pool : open_pools) {
        vkDestroyDescriptorPool(device->logical_device, pool, 0);
    }
    open_pools.clear();
    for (auto pool : full_pools) {
        vkDestroyDescriptorPool(device->logical_device, pool, 0);
    }
    full_pools.clear();
}

void DescriptorAllocator::reset_all_descriptor_sets() {
    for (auto pool : open_pools) {
        vkResetDescriptorPool(device->logical_device, pool, 0);
    }
    for (auto pool : full_pools) {
        vkResetDescriptorPool(device->logical_device, pool, 0);
        open_pools.push_back(pool);
    }
    full_pools.clear();
}

VkDescriptorPool DescriptorAllocator::new_pool(uint32_t set_count, std::span<PoolSizeRatio> pool_size_ratios)
{
	std::vector<VkDescriptorPoolSize> pool_sizes;
	for (PoolSizeRatio ratio : pool_size_ratios) {
		pool_sizes.push_back(VkDescriptorPoolSize{
			.type = ratio.type,
			.descriptorCount = uint32_t(ratio.ratio * set_count)
		});
	}

	VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
        .flags = pool_flags,
        .maxSets = set_count,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

	VkDescriptorPool pool;
	if (vkCreateDescriptorPool(device->logical_device, &pool_info, nullptr, &pool)) {
        Logger::logError("Failed to create descriptor pool!");
    }

    open_pools.push_back(pool);
    return pool;
}

VkDescriptorPool DescriptorAllocator::get_open_pool() {

    VkDescriptorPool pool;
    if (open_pools.size() != 0) { // There is an open pool available
        pool = open_pools.back();
    }
    else {
	    pool = new_pool(sets_per_pool, pool_size_ratios);

	    sets_per_pool = sets_per_pool * 1.5;
	    if (sets_per_pool > MAX_SETS) {
		    sets_per_pool = MAX_SETS;
	    }
    }
    return pool;
}

VkDescriptorSet DescriptorAllocator::allocate_descriptor_set(VkDescriptorSetLayout layout) {

    VkDescriptorPool pool_to_use = get_open_pool();

	VkDescriptorSetAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = pool_to_use,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout
	};

	VkDescriptorSet set;
	VkResult result = vkAllocateDescriptorSets(device->logical_device, &alloc_info, &set);

    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        // Try again with a new pool
        full_pools.push_back(pool_to_use);
        open_pools.pop_back();
        pool_to_use = get_open_pool();
        alloc_info.descriptorPool = pool_to_use;

        if (vkAllocateDescriptorSets(device->logical_device, &alloc_info, &set) != VK_SUCCESS) {
            // This time there is something else wrong
            Logger::logError("Failed to allocate descriptor set!");
        }
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
    this->descriptor_allocator.initialize(&renderer->device, max_sets, pool_size_ratios);
    this->descriptor_layout_builder.initialize(&renderer->device);
    this->descriptor_writer.initialize(&renderer->device);
}

void DescriptorBuilder::cleanup() {
    descriptor_allocator.cleanup();
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
    set.handle = descriptor_allocator.allocate_descriptor_set(set.layout);
    descriptor_writer.write(set.handle);

    return set;
}
