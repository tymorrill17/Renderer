#include "renderer/image.h"
#include "utility/allocator.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"

// Image --------------------------------------------------------------------------------------------------

Image::Image(VkImage image, VkImageView imageView, VkExtent3D extent, VkFormat format, VkImageLayout imageLayout) :
	_image(image),
    _imageView(imageView),
    _imageLayout(imageLayout),
    _extent(extent),
    _format(format) {
}

Image::~Image() {}

Image::Image(Image&& other) noexcept :
    _image(std::move(other._image)),
    _imageView(std::move(other._imageView)),
    _imageLayout(std::move(other._imageLayout)),
    _extent(std::move(other._extent)),
    _format(std::move(other._format)) {

    other._image = VK_NULL_HANDLE;
    other._imageView = VK_NULL_HANDLE;
    other._imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    other._extent = {0, 0};
    other._format = VK_FORMAT_UNDEFINED;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        _image = std::move(other._image);
        _imageView = std::move(other._imageView);
        _imageLayout = std::move(other._imageLayout);
        _extent = std::move(other._extent);
        _format = std::move(other._format);
        other._image = VK_NULL_HANDLE;
        other._imageView = VK_NULL_HANDLE;
        other._imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        other._extent = {0, 0};
        other._format = VK_FORMAT_UNDEFINED;
    }
    return *this;
}


void Image::transitionImage(Command& cmd, VkImageLayout newLayout) {
	VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageSubresourceRange subresourceRange{
		.aspectMask = aspectMask,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageMemoryBarrier2 imageBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
		.oldLayout = _imageLayout,
		.newLayout = newLayout,
		.image = _image,
		.subresourceRange = subresourceRange
	};

	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.pNext = nullptr,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &imageBarrier
	};

	vkCmdPipelineBarrier2(cmd.buffer(), &dependencyInfo);

	_imageLayout = newLayout;
}

void Image::copyImageOnGPU(Command& cmd, Image* src, Image* dst) {
	VkImageBlit2 blitRegion{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr
	};

	blitRegion.srcOffsets[1].x = src->extent().width;
	blitRegion.srcOffsets[1].y = src->extent().height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dst->extent().width;
	blitRegion.dstOffsets[1].y = dst->extent().height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo{
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = nullptr,
		.srcImage = src->image(),
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstImage = dst->image(),
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &blitRegion,
		.filter = VK_FILTER_LINEAR
	};

	vkCmdBlitImage2(cmd.buffer(), &blitInfo);
}

VkRenderingAttachmentInfoKHR Image::attachmentInfo(VkImageView imageView, VkClearValue* pClear, VkImageLayout imageLayout) {
	VkRenderingAttachmentInfoKHR renderingAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
		.pNext = nullptr,
		.imageView = imageView,
		.imageLayout = imageLayout,
		.loadOp = pClear != VK_NULL_HANDLE ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE
	};
	if (pClear) renderingAttachmentInfo.clearValue = *pClear;
	return renderingAttachmentInfo;
}

// AllocatedImage --------------------------------------------------------------------------------------------------

void AllocatedImage::createAllocatedImage() {
	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.imageType = VK_IMAGE_TYPE_2D, // Need to change this if I need 3D images
		.format = _format,
		.extent = _extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT, // Only applicable for target images
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = _usageFlags
	};

	VmaAllocationCreateInfo allocInfo{
		.usage = _memoryUsage,
		.requiredFlags = static_cast<VkMemoryPropertyFlags>(_vkMemoryUsage)
	};

	if (vmaCreateImage(_deviceMemoryManager->allocator(), &imageInfo, &allocInfo, &_image, &_allocation, nullptr) != VK_SUCCESS) {
        Logger::logError("Failed to create and allocate image!");
	}
    vmaSetAllocationName(_deviceMemoryManager->allocator(), _allocation, "AllocatedImage");

	VkImageSubresourceRange subresourceRange{
		.aspectMask = _aspectFlags,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageViewCreateInfo imageViewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.image = _image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = _format,
		.subresourceRange = subresourceRange
	};

	if (vkCreateImageView(_device->handle(), &imageViewInfo, nullptr, &_imageView) != VK_SUCCESS) {
        Logger::logError("Failed to create allocated image view!");
	}
}

AllocatedImage::AllocatedImage(Device* device, DeviceMemoryManager* deviceMemoryManager,
	VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags,
	VmaMemoryUsage memoryUsage, VkMemoryAllocateFlags vkMemoryUsage,
	VkImageAspectFlags aspectFlags) :
	Image(VK_NULL_HANDLE, VK_NULL_HANDLE, extent, format, VK_IMAGE_LAYOUT_UNDEFINED),
	_device(device), _deviceMemoryManager(deviceMemoryManager), _allocation(nullptr), _usageFlags(usageFlags),
	_memoryUsage(memoryUsage), _vkMemoryUsage(vkMemoryUsage), _aspectFlags(aspectFlags) {

	createAllocatedImage();
}

AllocatedImage::AllocatedImage(AllocatedImage&& other) noexcept :
    Image(std::move(other)),
    _device(std::move(other._device)),
    _deviceMemoryManager(std::move(other._deviceMemoryManager)),
    _allocation(std::move(other._allocation)),
    _usageFlags(std::move(other._usageFlags)),
    _memoryUsage(std::move(other._memoryUsage)),
    _vkMemoryUsage(std::move(other._vkMemoryUsage)),
    _aspectFlags(std::move(other._aspectFlags)) {

    other._device = nullptr;
    other._deviceMemoryManager = nullptr;
    other._allocation = nullptr;
}

AllocatedImage& AllocatedImage::operator=(AllocatedImage&& other) noexcept {
    if (this != &other) {
        Image::operator=(std::move(other));
        _device = std::move(other._device);
        _deviceMemoryManager = std::move(other._deviceMemoryManager);
        _allocation = std::move(other._allocation);
        _usageFlags = std::move(other._usageFlags);
        _memoryUsage = std::move(other._memoryUsage);
        _vkMemoryUsage = std::move(other._vkMemoryUsage);
        _aspectFlags = std::move(other._aspectFlags);

        other._device = nullptr;
        other._deviceMemoryManager = nullptr;
        other._allocation = nullptr;
    }
    return *this;
}

void AllocatedImage::cleanup() {
	vkDestroyImageView(_device->handle(), _imageView, nullptr);
	vmaDestroyImage(_deviceMemoryManager->allocator(), _image, _allocation);
}

void AllocatedImage::recreate(VkExtent3D extent) {
	cleanup();
	_extent = extent;
	_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_imageView = VK_NULL_HANDLE;
	createAllocatedImage();
}

// SwapchainImage --------------------------------------------------------------------------------------------------

SwapchainImage::SwapchainImage(Device* device, VkImage image, VkExtent3D extent,
	VkFormat format) :
	Image(image, VK_NULL_HANDLE, extent, format, VK_IMAGE_LAYOUT_UNDEFINED), _device(device) {

	// Create associated image view. This is going to be a color aspect image view
	VkImageSubresourceRange subresourceRange{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageViewCreateInfo imageViewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.image = _image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = _format,
		.subresourceRange = subresourceRange
	};

	if (vkCreateImageView(_device->handle(), &imageViewInfo, nullptr, &_imageView) != VK_SUCCESS) {
        Logger::logError("Failed to create swapchain image view!");
	}
}

SwapchainImage::SwapchainImage(SwapchainImage&& other) noexcept :
    Image(std::move(other)),
    _device(std::move(other._device)) {
    other._device = nullptr;
}

SwapchainImage& SwapchainImage::operator=(SwapchainImage&& other) noexcept {
    if (this != &other) {
        Image::operator=(std::move(other));
        _device = std::move(other._device);
        other._device = nullptr;
    }
    return *this;
}


SwapchainImage::~SwapchainImage() {
	vkDestroyImageView(_device->handle(), _imageView, nullptr);
}


