#include "renderer/image.h"
#include "utility/allocator.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"
#include "renderer/renderer.h"

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

void Image::copy_subimage(Command *cmd, ImageType *src, VkExtent3D src_extent, ImageType *dst, VkExtent3D dst_extent) {
	VkImageBlit2 blit_region{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr
	};

	blit_region.srcOffsets[1].x = src_extent.width;
	blit_region.srcOffsets[1].y = src_extent.height;
	blit_region.srcOffsets[1].z = src_extent.depth;

	blit_region.dstOffsets[1].x = dst_extent.width;
	blit_region.dstOffsets[1].y = dst_extent.height;
	blit_region.dstOffsets[1].z = dst_extent.depth;

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

void Image::copy_image(Command* cmd, ImageType* src, ImageType* dst) {
    // Copy subimage but use the entire image
    copy_subimage(cmd, src, src->extent, dst, dst->extent);
}

void Image::copy_data_to_image(ImageType *image, void *data, size_t pixel_bytes) {

    size_t data_size = image->extent.depth * image->extent.width * image->extent.height * pixel_bytes;
	Buffer upload_buffer = image->renderer->create_buffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    upload_buffer.write_data(data);

    image->renderer->immediate_command.run_command([&](Command* immediate_command) {
        Image::transition_image(immediate_command, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy_info{
		    .bufferOffset = 0,
		    .bufferRowLength = 0,
		    .bufferImageHeight = 0,
		    .imageExtent = image->extent,
        };

        // Just doing the base mipmap level for now.
        copy_info.imageSubresource.aspectMask = image->aspect_flags;
		copy_info.imageSubresource.mipLevel = 0;
		copy_info.imageSubresource.baseArrayLayer = 0;
		copy_info.imageSubresource.layerCount = image->mip_level_count;

		// copy the buffer into the image
		vkCmdCopyBufferToImage(immediate_command->buffer, upload_buffer.handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

		Image::transition_image(immediate_command, image,	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    upload_buffer.cleanup();
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

void AllocatedImage::cleanup() {
	vkDestroyImageView(renderer->device.logical_device, view, nullptr);
	vmaDestroyImage(renderer->device_memory_manager.allocator, handle, allocation);
}

void AllocatedImage::recreate(VkExtent3D extent) {
    bool use_mipmaps = this->mip_level_count > 1;
	cleanup();
    *this = std::move(renderer->create_image(extent, this->format, this->usage_flags, use_mipmaps));
    this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

// SwapchainImage --------------------------------------------------------------------------------------------------

void SwapchainImage::initialize(Renderer* renderer, VkImage image, VkExtent3D extent, VkFormat format) {

    this->renderer = renderer;
    this->handle = image;
    this->extent = extent;
    this->format = format;
    this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    this->aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
    this->mip_level_count = 1;

	// Create associated image view. This is going to be a color aspect image view
	VkImageSubresourceRange subresource_range{
		.aspectMask = this->aspect_flags,
		.baseMipLevel = 0,
		.levelCount = mip_level_count,
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

	if (vkCreateImageView(renderer->device.logical_device, &image_view_info, nullptr, &view) != VK_SUCCESS) {
        Logger::logError("Failed to create swapchain image view!");
	}

    this->render_semaphore.initialize(&renderer->device);
}

void SwapchainImage::cleanup() {
	vkDestroyImageView(renderer->device.logical_device, view, nullptr);
    render_semaphore.cleanup();
}


