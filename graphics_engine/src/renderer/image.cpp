#include "renderer/image.h"
#include "utility/allocator.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"

// Image --------------------------------------------------------------------------------------------------

void Image::transition_image(Command* cmd, ImageType* image, VkImageLayout new_layout) {
	VkImageAspectFlags aspect_mask = image->aspect_flags;
	VkImageSubresourceRange subresource_range{
		.aspectMask = aspect_mask,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageMemoryBarrier2 image_barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
		.oldLayout = image->layout,
		.newLayout = new_layout,
		.image = image->handle,
		.subresourceRange = subresource_range
	};

	VkDependencyInfo dependency_info{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.pNext = nullptr,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &image_barrier
	};

	vkCmdPipelineBarrier2(cmd->buffer, &dependency_info);

	image->layout = new_layout;
}

void Image::copy_image_to_GPU(Command* cmd, ImageType* src, ImageType* dst) {
	VkImageBlit2 blit_region{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr
	};

	blit_region.srcOffsets[1].x = src->extent.width;
	blit_region.srcOffsets[1].y = src->extent.height;
	blit_region.srcOffsets[1].z = 1;

	blit_region.dstOffsets[1].x = dst->extent.width;
	blit_region.dstOffsets[1].y = dst->extent.height;
	blit_region.dstOffsets[1].z = 1;

	blit_region.srcSubresource.aspectMask = src->aspect_flags;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcSubresource.mipLevel = 0;

	blit_region.dstSubresource.aspectMask = dst->aspect_flags;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blit_info{
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = nullptr,
		.srcImage = src->handle,
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstImage = dst->handle,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &blit_region,
		.filter = VK_FILTER_LINEAR
	};

	vkCmdBlitImage2(cmd->buffer, &blit_info);
}

VkRenderingAttachmentInfoKHR Image::color_attachment_info(VkImageView image_view, VkClearValue* clear_value, VkImageLayout image_layout) {
	VkRenderingAttachmentInfoKHR rendering_attachment_info{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
		.pNext = nullptr,
		.imageView = image_view,
		.imageLayout = image_layout,
		.loadOp = clear_value != VK_NULL_HANDLE ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE
	};
	if (clear_value) rendering_attachment_info.clearValue = *clear_value;
	return rendering_attachment_info;
}

VkRenderingAttachmentInfoKHR Image::depth_attachment_info(VkImageView image_view, VkImageLayout image_layout) {
	VkRenderingAttachmentInfoKHR depth_attachment_info{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
		.pNext = nullptr,
		.imageView = image_view,
		.imageLayout = image_layout,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	};
    depth_attachment_info.clearValue.depthStencil.depth = 0.0f; // Initialize the depth buffer to 0
	return depth_attachment_info;
}

// AllocatedImage --------------------------------------------------------------------------------------------------

void AllocatedImage::initialize(Device* device, DeviceMemoryManager* device_memory_manager,
	VkExtent3D extent, VkFormat format, VkImageUsageFlags usage_flags,
	VmaMemoryUsage vma_memory_usage, VkMemoryAllocateFlags vk_memory_usage,
	VkImageAspectFlags aspect_flags) {

    this->device                = device;
    this->device_memory_manager = device_memory_manager;
    this->usage_flags           = usage_flags;
    this->vma_memory_usage      = vma_memory_usage;
    this->vk_memory_usage       = vk_memory_usage;
    this->aspect_flags          = aspect_flags;
    this->extent                = extent;
    this->format                = format;
    this->layout                = VK_IMAGE_LAYOUT_UNDEFINED;

	create_image();
}

void AllocatedImage::cleanup() {
	vkDestroyImageView(device->logical_device, view, nullptr);
	vmaDestroyImage(device_memory_manager->allocator, handle, allocation);
}

void AllocatedImage::create_image() {
	VkImageCreateInfo image_info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.imageType = VK_IMAGE_TYPE_2D, // Need to change this if I need 3D images
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT, // Only applicable for target images
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage_flags
	};

	VmaAllocationCreateInfo alloc_info{
		.usage = vma_memory_usage,
		.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk_memory_usage)
	};

	if (vmaCreateImage(device_memory_manager->allocator, &image_info, &alloc_info, &handle, &allocation, nullptr) != VK_SUCCESS) {
        Logger::logError("Failed to create and allocate image!");
	}
    // vmaSetAllocationName(device_memory_manager->allocator, allocation, "AllocatedImage");

	VkImageSubresourceRange subresource_range{
		.aspectMask = aspect_flags,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageViewCreateInfo image_view_info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.image = handle,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = subresource_range
	};

	if (vkCreateImageView(device->logical_device, &image_view_info, nullptr, &view) != VK_SUCCESS) {
        Logger::logError("Failed to create allocated image view!");
	}
}

void AllocatedImage::recreate(VkExtent3D extent) {
	cleanup();
	this->extent = extent;
    this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_image();
}

// SwapchainImage --------------------------------------------------------------------------------------------------

void SwapchainImage::initialize(Device* device, VkImage image, VkExtent3D extent, VkFormat format) {

    this->device = device;
    this->handle = image;
    this->extent = extent;
    this->format = format;
    this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    this->aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;

	// Create associated image view. This is going to be a color aspect image view
	VkImageSubresourceRange subresource_range{
		.aspectMask = this->aspect_flags,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageViewCreateInfo image_view_info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.image = handle,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = subresource_range
	};

	if (vkCreateImageView(device->logical_device, &image_view_info, nullptr, &view) != VK_SUCCESS) {
        Logger::logError("Failed to create swapchain image view!");
	}
}

void SwapchainImage::cleanup() {
	vkDestroyImageView(device->logical_device, view, nullptr);
}


