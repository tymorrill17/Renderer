#pragma once
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "utility/allocator.h"
#include "renderer/command.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>

class Renderer;

// Generic Image type to use in image operations below like transisitoning and copying to GPU
class ImageType {
public:
    VkImage handle;
    VkImageView view;
    VkImageLayout layout;
    VkExtent3D extent;
    VkFormat format;
	VkImageAspectFlags aspect_flags;
    uint32_t mip_level_count;

    Renderer* renderer;
};

namespace Image {
    void transition_image(Command* cmd, ImageType* image, VkImageLayout new_layout);
	void copy_image(Command* cmd, ImageType* src, ImageType* dst);
	void copy_subimage(Command* cmd, ImageType* src, VkExtent3D src_extent, ImageType* dst, VkExtent3D dst_extent);
	VkRenderingAttachmentInfoKHR color_attachment_info(VkImageView image_view, VkClearValue* clear_value, VkImageLayout image_layout);
	VkRenderingAttachmentInfoKHR depth_attachment_info(VkImageView image_view, VkImageLayout image_layout);
};

class AllocatedImage : public ImageType {
public:
    void cleanup();
    void recreate(VkExtent3D extent);

    VmaAllocation allocation;

	VkImageUsageFlags usage_flags;
	VmaMemoryUsage vma_memory_usage;
	VkMemoryAllocateFlags vk_memory_usage;
};

class SwapchainImage : public ImageType {
public:
    void initialize(Renderer* renderer, VkImage image, VkExtent3D extent, VkFormat format);
    void cleanup();
};
