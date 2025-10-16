#pragma once
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "utility/allocator.h"
#include "renderer/command.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"

// Generic Image type to use in image operations below like transisitoning and copying to GPU
class ImageType {
public:
    VkImage handle;
    VkImageView view;
    VkImageLayout layout;
    VkExtent3D extent;
    VkFormat format;
};

namespace Image {
    void transition_image(Command* cmd, ImageType* image, VkImageLayout new_layout);
	void copy_image_to_GPU(Command* cmd, ImageType* src, ImageType* dst);
	VkRenderingAttachmentInfoKHR attachment_info(VkImageView image_view, VkClearValue* clear_value, VkImageLayout image_layout);
};

class DrawImage : public ImageType {
public:
    void initialize(Device* device, DeviceMemoryManager* device_memory_manager,
		VkExtent3D extent, VkFormat format, VkImageUsageFlags usage_flags,
		VmaMemoryUsage memory_usage, VkMemoryAllocateFlags vk_memory_usage,
		VkImageAspectFlags aspect_flags);
    void cleanup();
    void recreate(VkExtent3D extent);

    Device* device;
    DeviceMemoryManager* device_memory_manager;
    VmaAllocation allocation;

	VkImageUsageFlags usage_flags;
	VmaMemoryUsage vma_memory_usage;
	VkMemoryAllocateFlags vk_memory_usage;
	VkImageAspectFlags aspect_flags;

private:
    void create_image();
};

class SwapchainImage : public ImageType {
public:
    void initialize(Device* device, VkImage image, VkExtent3D extent, VkFormat format);
    void cleanup();

    Device* device;
};
